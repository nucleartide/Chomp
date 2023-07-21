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

TArray<FName> AMovablePawn::GetTagsToCollideWith()
{
	return TagsToCollideWith;
}

FMovementResult AMovablePawn::MoveTowardsPoint(FGridLocation TargetGridPosition, FGridLocation TargetDirection, float DeltaTime)
{
	// Keep a reference to the level instance.
	auto LevelInstance = ULevelLoader::GetInstance(Level);

	// Then, move in the TargetDirection.
	{
		FVector DeltaLocation{TargetDirection.X, TargetDirection.Y, 0.0f};
		DeltaLocation *= MovementSpeed * DeltaTime;
		AddActorWorldOffset(DeltaLocation, false);
	}

	// Check if we moved past the target.
	FMovementResult Result;
	{
		auto TargetWorldPosition = LevelInstance->GridToWorld(TargetGridPosition);
		auto ActorLocation2D = GetActorLocation2D(this);
		FVector2D TargetDirectionVec{TargetDirection.X, TargetDirection.Y};
		auto MovementDotProduct = FVector2D::DotProduct(TargetDirectionVec, (TargetWorldPosition - ActorLocation2D).GetSafeNormal());
		auto AmountDotProduct = FVector2D::DotProduct(TargetDirectionVec, TargetWorldPosition - ActorLocation2D);
		Result.MovedPastTarget = FMath::Abs(MovementDotProduct + 1) < 0.1f;
		Result.AmountMovedPast = FMath::Abs(AmountDotProduct);
	}

	// Rotate toward target.
	{
		// Get current rotation.
		auto ActorRotation = GetActorRotation();

		// Get target rotation.
		auto ActorLocation = GetActorLocation();
		FVector DeltaLocation2(TargetDirection.X, TargetDirection.Y, 0.0f);
		auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, ActorLocation + DeltaLocation2);

		// Lerp to target rotation.
		auto NewRotation = FMath::RInterpTo(ActorRotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

		// Set rotation to interpolated rotation value.
		SetActorRotation(NewRotation);
	}

	WrapAroundWorld();

	return Result;
}

void AMovablePawn::WrapAroundWorld()
{
	// Grab references to stuff.
	auto LevelHeight = ULevelLoader::GetInstance(Level)->GetLevelHeight();
	auto LevelWidth = ULevelLoader::GetInstance(Level)->GetLevelWidth();
	auto Location = GetActorLocation();

	// Get bottom-left corner tile.
	FGridLocation BottomLeft{0, 0};
	auto BottomLeftWorldPos = ULevelLoader::GetInstance(Level)->GridToWorld(BottomLeft);
	auto BottomBound = BottomLeftWorldPos.X - 50.0f;
	auto LeftBound = BottomLeftWorldPos.Y - 50.0f;

	// Get top-right corner tile.
	FGridLocation TopRight{LevelHeight - 1, LevelWidth - 1};
	auto TopRightWorldPos = ULevelLoader::GetInstance(Level)->GridToWorld(TopRight);
	auto TopBound = TopRightWorldPos.X + 50.0f;
	auto RightBound = TopRightWorldPos.Y + 50.0f;

	// Update X component of Location if needed.
	if (Location.X < BottomBound)
	{
		auto Diff = BottomBound - Location.X;
		Location.X = TopBound - Diff;
	}
	else if (Location.X > TopBound)
	{
		auto Diff = Location.X - TopBound;
		Location.X = BottomBound + Diff;
	}

	// Update Y component of Location if needed.
	if (Location.Y < LeftBound)
	{
		auto Diff = LeftBound - Location.Y;
		Location.Y = RightBound - Diff;
	}
	else if (Location.Y > RightBound)
	{
		auto Diff = Location.Y - RightBound;
		Location.Y = LeftBound + Diff;
	}

	// Update actor location.
	SetActorLocation(Location);
}
