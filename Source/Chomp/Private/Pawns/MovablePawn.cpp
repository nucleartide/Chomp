#include "MovablePawn.h"

#include "AStar/MovementPath.h"
#include "Constants/ChompGameplayTag.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Movement/Movement.h"
#include "Movement/MovementIntention.h"
#include "Utils/SafeGet.h"

AMovablePawn::AMovablePawn(): APawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

// move toward an FGridLocation, and return a movement result.
FMoveInDirectionResult AMovablePawn::MoveInDirection(
	TSharedPtr<FMovement> Movement,
	TSharedPtr<FMovementIntention> MovementIntention,
	const float DeltaTime) const
{
	// Preconditions.
	check(Movement->Direction.IsNonZero());

	// Can't move if there is no target, cause then we can't perform axis alignment.
	if (!Movement->HasValidTargetTile())
		return FMoveInDirectionResult{GetActorLocation(), GetActorRotation(), false};

	// Otherwise, move in the TargetDirection.
	auto ActorLocation = GetActorLocation();
	{
		FVector DeltaLocation{static_cast<double>(Movement->Direction.X), static_cast<double>(Movement->Direction.Y), 0.0};
		DeltaLocation *= MovementSpeed * DeltaTime;
		ActorLocation += DeltaLocation;
	}

	// Check if we moved past the target.
	bool MovedPastTarget = false;
	{
		const auto LevelInstance = ULevelLoader::GetInstance(Level);
		FVector2D Dir{static_cast<double>(Movement->Direction.X), static_cast<double>(Movement->Direction.Y)};
		const auto TargetLocation = LevelInstance->GridToWorld(Movement->TargetTile.GridLocation);
		const auto ActorLocation2D = FVector2D{ActorLocation.X, ActorLocation.Y};
		const auto MovementDotProduct = FVector2D::DotProduct(Dir, (TargetLocation - ActorLocation2D).GetSafeNormal());
		const auto AmountDotProduct = FVector2D::DotProduct(Dir, TargetLocation - ActorLocation2D);
		MovedPastTarget = FMath::Abs(MovementDotProduct + 1) < 0.1f;
		const auto AmountMovedPast = FMath::Abs(AmountDotProduct);

		// If we did move past the target,
		if (MovedPastTarget)
		{
			// Then if we can travel in the intended direction,
			if (CanTravelInDirection(ActorLocation, MovementIntention->Direction))
			{
				// Apply the remaining delta toward the next grid location's direction.
				ActorLocation = FVector{
					TargetLocation.X + MovementIntention->Direction.X * AmountMovedPast,
					TargetLocation.Y + MovementIntention->Direction.Y * AmountMovedPast,
					0.0f
				};
			}
			// Then if we can travel in the current direction,
			else if (CanTravelInDirection(ActorLocation, Movement->Direction))
			{
				// Apply the remaining delta toward the next grid location's direction.
				ActorLocation = FVector{
					TargetLocation.X + Movement->Direction.X * AmountMovedPast,
					TargetLocation.Y + Movement->Direction.Y * AmountMovedPast,
					0.0f
				};
			}
			// Otherwise,
			else
			{
				// Lock the location to the target's location.
				ActorLocation = FVector{TargetLocation.X, TargetLocation.Y, 0.0};
			}
		}
	}

	// Finally, perform wrap-around.
	ActorLocation = WrapAroundWorld(ActorLocation);

	// And very finally, compute new rotation.
	// You can extract the part of AI movement into a new method, and just call that.
	const auto ActorRotation = ComputeNewRotation(GetActorLocation(), ActorLocation, DeltaTime);

	// Return the final computed location and rotation.
	return FMoveInDirectionResult{ActorLocation, ActorRotation, MovedPastTarget};
}

FGridLocation AMovablePawn::GetGridLocation() const
{
	const auto ActorLocation = GetActorLocation();
	const FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
	return ULevelLoader::GetInstance(Level)->WorldToGrid(ActorLocation2D);
}

FVector2D AMovablePawn::GetActorLocation2D() const
{
	const auto Location = GetActorLocation();
	const FVector2D Location2D{Location.X, Location.Y};
	return Location2D;
}

FMovementResult AMovablePawn::MoveAlongPath(
	FMovementPath* MovementPath,
	const float DeltaTime) const
{
	// If we're already at the end, return a no-movement result. Note that no rotation takes place.
	const auto Location = GetActorLocation();
	const auto Rotation = GetActorRotation();
	if (MovementPath->WasCompleted(Location))
		return FMovementResult{Location, Rotation};

	// Else, compute the DeltaDistance.
	const auto DeltaDistance = MovementSpeed * DeltaTime;

	// Call out to MovementPath->MoveAlongPath(ActorLocation, DeltaDistance), which will return an FVector.
	const auto NewLocation = MovementPath->MoveAlongPath(Location, DeltaDistance);

	// Compute new rotation given the new position.
	const auto Dir = (NewLocation - Location).GetSafeNormal();
	const auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Location, Location + Dir);
	const auto NewRotation = FMath::RInterpTo(Rotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

	// Return computed results.
	return FMovementResult{NewLocation, NewRotation};
}

bool AMovablePawn::CanTravelInDirection(FVector Location, FGridLocation Direction) const
{
	// Prepare data needed for performing our sweep check.
	// Diameter needs to be slightly less than 100.0f to avoid overlapping with adjacent wall tiles.
	const auto [X, Y] = Direction;
	constexpr auto ActorDiameter = 90.0f;
	constexpr auto ActorRadius = ActorDiameter * 0.5f;
	const auto ActorSphere = FCollisionShape::MakeSphere(ActorRadius);
	const auto StartLocation = Location;
	const auto EndLocation = Location + FVector{X * ActorDiameter, Y * ActorDiameter, 0.0f};
	const auto WorldInstance = FSafeGet::World(this);

	// Perform sweep check to see if we overlap with anything in Direction's way.
	TArray<FHitResult> HitResults;
	WorldInstance->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Visibility,
		ActorSphere);

	// If we overlapped with something, then we can't travel in Direction's way. Return nothing.
	for (auto HitResult : HitResults)
		if (auto HitActor = HitResult.GetActor(); FChompGameplayTag::ActorHasOneOf(HitActor, TagsToCollideWith))
			return false;

	// Otherwise, we *can* travel in Direction's way. Return true.
	return true;
}

FVector AMovablePawn::WrapAroundWorld(FVector Location) const
{
	// Grab references to stuff.
	const auto LevelHeight = ULevelLoader::GetInstance(Level)->GetLevelHeight();
	const auto LevelWidth = ULevelLoader::GetInstance(Level)->GetLevelWidth();

	// Get bottom-left corner tile.
	constexpr FGridLocation BottomLeft{0, 0};
	const auto BottomLeftWorldPos = ULevelLoader::GetInstance(Level)->GridToWorld(BottomLeft);
	const auto BottomBound = BottomLeftWorldPos.X - 50.0f;
	const auto LeftBound = BottomLeftWorldPos.Y - 50.0f;

	// Get top-right corner tile.
	const FGridLocation TopRight{LevelHeight - 1, LevelWidth - 1};
	const auto TopRightWorldPos = ULevelLoader::GetInstance(Level)->GridToWorld(TopRight);
	const auto TopBound = TopRightWorldPos.X + 50.0f;
	const auto RightBound = TopRightWorldPos.Y + 50.0f;

	// Update X component of Location if needed.
	if (Location.X < BottomBound)
	{
		const auto Diff = BottomBound - Location.X;
		Location.X = TopBound - Diff;
	}
	else if (Location.X > TopBound)
	{
		const auto Diff = Location.X - TopBound;
		Location.X = BottomBound + Diff;
	}

	// Update Y component of Location if needed.
	if (Location.Y < LeftBound)
	{
		const auto Diff = LeftBound - Location.Y;
		Location.Y = RightBound - Diff;
	}
	else if (Location.Y > RightBound)
	{
		const auto Diff = Location.Y - RightBound;
		Location.Y = LeftBound + Diff;
	}

	// Return wrapped-around Location.
	return Location;
}

FRotator AMovablePawn::ComputeNewRotation(const FVector& Location, const FVector& NewLocation, float DeltaTime) const
{
	const auto Rotation = GetActorRotation();
	const auto Dir = (NewLocation - Location).GetSafeNormal();
	const auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Location, Location + Dir);
	const auto NewRotation = FMath::RInterpTo(Rotation, LookAtRotation, DeltaTime, RotationInterpSpeed);
	return NewRotation;
}
