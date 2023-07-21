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

	return Result;

	// TODO: add this back.
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
