#include "MovablePawn.h"

#include "AStar/MovementPath.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

AMovablePawn::AMovablePawn(): APawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

TArray<FName> AMovablePawn::GetTagsToCollideWith()
{
	return TagsToCollideWith;
}

// move toward an FGridLocation, and return a movement result.
FMovementResult AMovablePawn::MoveTowardsPoint(
	const FGridLocation& TargetGridPosition,
	const FGridLocation& TargetDirection,
	const float DeltaTime,
	FName DebugLabel)
{
	FMovementResult Result{false, 0.0f};
	if (TargetDirection.IsZero())
		return Result;

	// Keep a reference to the level instance.
	const auto LevelInstance = ULevelLoader::GetInstance(Level);

	// Then, move in the TargetDirection.
	{
		FVector DeltaLocation{static_cast<double>(TargetDirection.X), static_cast<double>(TargetDirection.Y), 0.0f};
		DeltaLocation *= MovementSpeed * DeltaTime;
		AddActorWorldOffset(DeltaLocation, false);
	}

	// Check if we moved past the target.
	{
		const auto TargetWorldPosition = LevelInstance->GridToWorld(TargetGridPosition);
		const auto ActorLocation2D = GetActorLocation2D();
		const FVector2D TargetDirectionVec{
			static_cast<double>(TargetDirection.X), static_cast<double>(TargetDirection.Y)
		};
		const auto MovementDotProduct = FVector2D::DotProduct(TargetDirectionVec,
		                                                      (TargetWorldPosition - ActorLocation2D).GetSafeNormal());
		const auto AmountDotProduct = FVector2D::DotProduct(TargetDirectionVec, TargetWorldPosition - ActorLocation2D);
		// location
		// rotation
		Result.MovedPastTarget = FMath::Abs(MovementDotProduct + 1) < 0.1f;
		// MovementDotProduct moved past target if it's around -1.0f
		Result.AmountMovedPast = FMath::Abs(AmountDotProduct);
	}

	// Rotate toward target.
	{
		// Get current rotation.
		const auto ActorRotation = GetActorRotation();

		// Get target rotation.
		const auto ActorLocation = GetActorLocation();
		const FVector DeltaLocation(TargetDirection.X, TargetDirection.Y, 0.0f);
		const auto LookAtRotation =
			UKismetMathLibrary::FindLookAtRotation(ActorLocation, ActorLocation + DeltaLocation);

		// Lerp to target rotation.
		const auto NewRotation = FMath::RInterpTo(ActorRotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

		// Set rotation to interpolated rotation value.
		SetActorRotation(NewRotation);
	}

	// TODO: this throws off your results, movedpasttarget and amountmovedpast has undefined behavior
	WrapAroundWorld();
	return Result;
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

// Note: I think it makes sense to have 2 movement algorithms for now.
// One is a cleaner version of the original, and I will refactor the player movement if the need arises.
FAIMovementResult AMovablePawn::MoveTowardsPoint2(
	const FVector& Location,
	const FRotator& Rotation,
	FMovementPath* MovementPath,
	const float DeltaTime) const
{
	// If we're already at the end, return a no-movement result. Note that no rotation takes place.
	if (MovementPath->WasCompleted(Location))
		 return FAIMovementResult{Location, Rotation};

	// Else, compute the DeltaDistance.
	const auto DeltaDistance = MovementSpeed * DeltaTime;

	// Call out to MovementPath->MoveAlongPath(ActorLocation, DeltaDistance), which will return an FVector.
	const auto NewLocation = MovementPath->MoveAlongPath(Location, DeltaDistance);

	// Compute new rotation given the new position.
	const auto Dir = (NewLocation - Location).GetSafeNormal();
	const auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Location, Location + Dir);
	const auto NewRotation = FMath::RInterpTo(Rotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

	// Return computed results.
	return FAIMovementResult{NewLocation, NewRotation};
}

void AMovablePawn::WrapAroundWorld()
{
	// Grab references to stuff.
	const auto LevelHeight = ULevelLoader::GetInstance(Level)->GetLevelHeight();
	const auto LevelWidth = ULevelLoader::GetInstance(Level)->GetLevelWidth();
	auto Location = GetActorLocation();

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

	// Update actor location.
	SetActorLocation(Location);
}
