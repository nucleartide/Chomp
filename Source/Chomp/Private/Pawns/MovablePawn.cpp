#include "MovablePawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Constants/GameplayTag.h"
#include "Utils/Debug.h"
#include "Utils/Actor.h"

AMovablePawn::AMovablePawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

BlockingEntity AMovablePawn::GetExcludedEntities()
{
	return ExcludedEntities;
}

bool AMovablePawn::MoveTowardsPoint(FGridLocation TargetGridPosition, float DeltaTime)
{
	return false;

	// Something to think about.
	// void WrapAroundWorld();

	// Old implementation.
#if false
	if (MovementDirection.X == 0 && MovementDirection.Y == 0)
		return;

	// Can't move diagonally in this game.
	check((MovementDirection.X == 0 && MovementDirection.Y == 0) ||
		  (MovementDirection.X != MovementDirection.Y));

	// Keep a reference to the level instance.
	auto LevelInstance = ULevelLoader::GetInstance(Level);

	// Get target grid position.
	FGridLocation TargetGridPosition;
	{
		auto ActorLocation = GetActorLocation();
		FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
		TargetGridPosition = LevelInstance->WorldToTargetGrid(ActorLocation2D, MovementDirection);
	}

	// If target grid position is not reachable, early return.
	if (!LevelInstance->IsPassable(TargetGridPosition, ExcludedEntities))
		return;

	// Otherwise, move in the specified MovementDirection.
	FVector DeltaLocation{MovementDirection.X, MovementDirection.Y, 0.0f};
	DeltaLocation *= MovementSpeed * DeltaTime;
	AddActorWorldOffset(DeltaLocation, false);

	// If we moved past the target AND the next node in MovementDirection isn't passable,
	bool MovedPastTarget = false;
	auto TargetWorldPosition = LevelInstance->GridToWorld(TargetGridPosition);
	{
		auto ActorLocation2D = GetActorLocation2D(this);
		FVector2D MovementDirectionVec{MovementDirection.X, MovementDirection.Y};
		auto MovementDotProduct = FVector2D::DotProduct(MovementDirectionVec, (TargetWorldPosition - ActorLocation2D).GetSafeNormal());
		MovedPastTarget = FMath::Abs(MovementDotProduct + 1) < 0.1f;
	}
	bool IsNextTargetPassable = false;
	{
		FGridLocation NextTargetGridPosition{TargetGridPosition.X + MovementDirection.X, TargetGridPosition.Y + MovementDirection.Y};
		IsNextTargetPassable = LevelInstance->IsPassable(NextTargetGridPosition, ExcludedEntities);
	}
	if (MovedPastTarget && !IsNextTargetPassable)
	{
		// Move actor back to TargetGridPosition (converted to world coords, of course).
		FVector TargetWorldPos{TargetWorldPosition.X, TargetWorldPosition.Y, 0.0f};
		SetActorLocation(TargetWorldPos);
	}
#endif

	// Rotation.
#if false
	if (MovementDirection.X != 0 || MovementDirection.Y != 0)
	{
		// Get current rotation.
		auto ActorRotation = GetActorRotation();

		// Get target rotation.
		auto ActorLocation = GetActorLocation();
		FVector DeltaLocation2(MovementDirection.X, MovementDirection.Y, 0.0f);
		auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, ActorLocation + DeltaLocation2);

		// Lerp to target rotation.
		auto NewRotation = FMath::RInterpTo(ActorRotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

		// Set rotation to interpolated rotation value.
		SetActorRotation(NewRotation);
	}
#endif

	// Wrapping mechanic.
#if false
void AMovablePawn::WrapAroundWorld()
{
	// Grab references to stuff.
	auto LevelHeight = ULevelLoader::GetInstance(Level)->GetLevelHeight();
	auto LevelWidth = ULevelLoader::GetInstance(Level)->GetLevelWidth();
	auto Location = GetActorLocation();

	// Update X component of Location if needed.
	auto HalfHeight = LevelHeight * 0.5f * 100.0f;
	if (Location.X < -HalfHeight)
	{
		auto Diff = -HalfHeight - Location.X;
		Location.X = HalfHeight - Diff;
	}
	else if (Location.X > HalfHeight)
	{
		auto Diff = Location.X - HalfHeight;
		Location.X = -HalfHeight + Diff;
	}

	// Update Y component of Location if needed.
	auto HalfWidth = LevelWidth * 0.5f * 100.0f;
	if (Location.Y < -HalfWidth)
	{
		auto Diff = -HalfWidth - Location.Y;
		Location.Y = HalfWidth - Diff;
	}
	else if (Location.Y > HalfWidth)
	{
		auto Diff = Location.Y - HalfWidth;
		Location.Y = -HalfWidth + Diff;
	}

	// Update actor location.
	SetActorLocation(Location);
}
#endif
}
