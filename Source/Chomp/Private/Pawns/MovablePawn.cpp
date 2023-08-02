#include "MovablePawn.h"

#include "AStar/MovementPath.h"
#include "Constants/ChompGameplayTag.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Movement/Movement.h"
#include "Movement/MovementIntention.h"
#include "Utils/MathHelpers.h"
#include "Utils/SafeGet.h"

AMovablePawn::AMovablePawn(): APawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Given two vectors From and To,
// find the difference vector (To - From) given that both vectors lie on a wrap-around space.
FVector2D AMovablePawn::MinDifferenceVector(FVector From, FVector To, const ULevelLoader* LevelInstance)
{
	// Wrap around both vectors so we know they are within bounds.
	From = WrapAroundWorld(From, LevelInstance);
	To = WrapAroundWorld(To, LevelInstance);

	const auto LevelHeight = LevelInstance->GetLevelHeight() * 100.0;
	const auto LevelWidth = LevelInstance->GetLevelWidth() * 100.0;

	// If you draw out individual axes, it should make sense.
	// We are shifting to a positive space, taking the modulo, then unshifting to the old space.
	// Note that fmod does not work the same as % in C++, so we need to adjust the result.
	const auto DiffX = MathHelpers::NotStupidFmod(To.X - From.X + LevelHeight * 0.5, LevelHeight) - LevelHeight * 0.5;
	const auto IntermediateResult = MathHelpers::NotStupidFmod(To.Y - From.Y + LevelWidth * 0.5, LevelWidth);
	const auto DiffY = IntermediateResult - LevelWidth * 0.5;
	check(FMath::Abs(DiffY) < 2000.0f);
	return FVector2D{DiffX, DiffY};
}

// Compute the dot product between MovementDirection and (Target - PostMovementLocation),
// which determines whether we've moved past the target or not, and by how much.
FPeriodicDotProductResult AMovablePawn::ComputeDotProduct(
	const FGridLocation& MovementDirection,
	const FVector& PostMovementLocation,
	const FGridLocation& Target,
	const ULevelLoader* LevelInstance)
{
	// Compute dot product.
	const FVector2D Dir{static_cast<double>(MovementDirection.X), static_cast<double>(MovementDirection.Y)};
	const auto TargetWorld2D = LevelInstance->GridToWorld(Target);
	const FVector TargetWorld{TargetWorld2D.X, TargetWorld2D.Y, 0.0};
	const auto MinDiff = MinDifferenceVector(PostMovementLocation, TargetWorld, LevelInstance);
	const auto DotProduct = FVector2D::DotProduct(Dir, MinDiff);

	// Sanity check. Can't be more than 100 cm away from Target.
	check(FMath::Abs(DotProduct) < 150.0f);

	// Return results.
	const auto MovedPastTarget = DotProduct < 0.0;
	const auto AmountMovedPast = DotProduct < 0.0 ? FMath::Abs(DotProduct) : 0.0;
	return FPeriodicDotProductResult{MovedPastTarget, AmountMovedPast};
}

FMoveInDirectionResult AMovablePawn::MoveInDirection(
	TSharedPtr<FMovement> Movement,
	TSharedPtr<FMovementIntention> MovementIntention,
	const float DeltaTime) const
{
	// Movement's Direction should always be non-zero. Pacman is always moving.
	check(Movement->Direction.IsNonZero());
	check(Movement->HasValidTargetTile());

	// If we move more than 100 cm, we have a problem because we may have moved into a wall.
	// In this case, reject moving until framerate is better.
	if (MovementSpeed * DeltaTime > 100.0)
		return FMoveInDirectionResult(GetActorLocation(), GetActorRotation(), false, false);

	// Compute the new ActorLocation.
	const auto OldLocation = GetActorLocation();
	const auto DeltaLocation = MovementSpeed * DeltaTime * Movement->Direction.ToFVector();
	const auto WrappedLocation = WrapAroundWorld(
		OldLocation + DeltaLocation,
		ULevelLoader::GetInstance(Level)
	);

	// Check whether we moved past the target, and by how much.
	const auto [MovedPastTarget, AmountMovedPast] = ComputeDotProduct(
		Movement->Direction,
		WrappedLocation,
		Movement->TargetTile.GridLocation,
		ULevelLoader::GetInstance(Level)
	);

	// If we moved past the target,
	const auto TargetWorld2D = ULevelLoader::GetInstance(Level)->GridToWorld(Movement->TargetTile.GridLocation);
	const FVector TargetWorld{TargetWorld2D.X, TargetWorld2D.Y, 0.0};
	const FVector IntendedDir{
		static_cast<double>(MovementIntention->Direction.X),
		static_cast<double>(MovementIntention->Direction.Y),
		0.0
	};
	const FVector CurrentDir{
		static_cast<double>(Movement->Direction.X),
		static_cast<double>(Movement->Direction.Y),
		0.0
	};
	const auto CanTravelInIntendedDir =
		MovedPastTarget &&
		MovementIntention->Direction.IsNonZero() &&
		Movement->Direction != MovementIntention->Direction &&
		CanTravelInDirection(TargetWorld, MovementIntention->Direction);
	const auto NewLocation =
		CanTravelInIntendedDir
			? TargetWorld + AmountMovedPast * IntendedDir
			: MovedPastTarget && CanTravelInDirection(TargetWorld, Movement->Direction)
			? TargetWorld + AmountMovedPast * CurrentDir 
			: MovedPastTarget
			? TargetWorld
			: WrappedLocation;

	// Finally, compute new rotation. Be cognizant of Pac-Man's wrap-around! May need to do modular arithmetic.
	const auto ActorRotation = ComputeNewRotation(GetActorLocation(), NewLocation, DeltaTime);

#if false
	// too hard to fix
	{
		// Grid alignment check.
		const auto Loc = NewLocation;
		check(
			FMath::IsNearlyEqual(MathHelpers::NotStupidFmod(Loc.X, 100.0), 0.0) ||
			FMath::IsNearlyEqual(MathHelpers::NotStupidFmod(Loc.Y, 100.0), 0.0)
		);
	}
#endif

	// And return the computed result.
	return FMoveInDirectionResult(NewLocation, ActorRotation, MovedPastTarget, CanTravelInIntendedDir);
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
	constexpr auto ActorDiameter = 95.0f;
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
	{
		const auto HitActor = HitResult.GetActor();
		if (FChompGameplayTag::ActorHasOneOf(HitActor, TagsToCollideWith))
			return false;
	}

	// Otherwise, we *can* travel in Direction's way. Return true.
	return true;
}

FVector AMovablePawn::WrapAroundWorld(FVector Location, const ULevelLoader* LevelInstance)
{
	// Grab references to stuff.
	const auto LevelHeight = LevelInstance->GetLevelHeight();
	const auto LevelWidth = LevelInstance->GetLevelWidth();

	// Get bottom-left corner tile.
	constexpr FGridLocation BottomLeft{0, 0};
	const auto BottomLeftWorldPos = LevelInstance->GridToWorld(BottomLeft);
	const auto BottomBound = BottomLeftWorldPos.X - 50.0;
	const auto LeftBound = BottomLeftWorldPos.Y - 50.0;

	// Get top-right corner tile.
	const FGridLocation TopRight{LevelHeight - 1, LevelWidth - 1};
	const auto TopRightWorldPos = LevelInstance->GridToWorld(TopRight);
	const auto TopBound = TopRightWorldPos.X + 50.0;
	const auto RightBound = TopRightWorldPos.Y + 50.0;

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

FRotator AMovablePawn::ComputeNewRotation(const FVector& CurrentLocation, const FVector& NewLocation,
                                          float DeltaTime) const
{
	const auto Rotation = GetActorRotation();
	const auto MinDiff2D = MinDifferenceVector(CurrentLocation, NewLocation, ULevelLoader::GetInstance(Level));
	const FVector MinDiff{MinDiff2D.X, MinDiff2D.Y, 0.0};
	const auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, CurrentLocation + MinDiff);
	const auto NewRotation = FMath::RInterpTo(Rotation, LookAtRotation, DeltaTime, RotationInterpSpeed);
	return NewRotation;
}
