#include "MovablePawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Constants/GameplayTag.h"
#include "Utils/Debug.h"

AMovablePawn::AMovablePawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMovablePawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMovablePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMovablePawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

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

void AMovablePawn::MoveVector(FVector2D Value, float DeltaTime)
{
	// Declare some variables.
	FVector DeltaLocation(Value.X, Value.Y, 0.0f);
	auto ActorScale = GetActorScale3D();
	float SphereDiameter = 100.0f * ActorScale.X;
	float SphereRadius = SphereDiameter * 0.5f * 0.5f; // Halve the radius a second time for a smaller collision sphere.
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	// Slide along horizontal walls
	if (DeltaLocation.X != 0.0f)
	{
		FVector StartLocation = GetActorLocation();
		FVector DeltaX = {DeltaLocation.X * Tolerance, 0.0f, 0.0f};
		FVector EndLocation = GetActorLocation() + DeltaX;

		TArray<FHitResult> HitResults;
		GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, SphereShape);

		for (auto HitResult : HitResults)
		{
			if (GameplayTag::ActorHasOneOf(HitResult.GetActor(), TagsToCollideWith))
			{
				DEBUG_LOG(TEXT("%s"), *(HitResult.GetActor())->GetName());
				DeltaLocation.X = 0;
			}
		}
	}

	// Slide along vertical walls
	if (DeltaLocation.Y != 0.0f)
	{
		FVector StartLocation = GetActorLocation();
		FVector DeltaY = {0.0f, DeltaLocation.Y * Tolerance, 0.0f};
		FVector EndLocation = GetActorLocation() + DeltaY;

		TArray<FHitResult> HitResults;
		GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Visibility, SphereShape);

		for (auto HitResult : HitResults)
		{
			if (GameplayTag::ActorHasOneOf(HitResult.GetActor(), TagsToCollideWith))
			{
				DeltaLocation.Y = 0;
			}
		}
	}

	// Let's apply the offset first.
	auto OldActorLocation = GetActorLocation();
	AddActorWorldOffset(DeltaLocation, false);

	// However, in the case where we're overlapping with a wall after applying the offset,
	if (DeltaLocation.X != 0.0f || DeltaLocation.Y != 0.0f)
	{
		// Perform overlap check.
		TArray<FOverlapResult> HitResults;
		GetWorld()->OverlapMultiByChannel(HitResults, GetActorLocation(), FQuat::Identity, ECC_Visibility, SphereShape);

		for (auto HitResult : HitResults)
		{
			// If the Actor is a wall,
			if (GameplayTag::ActorHasOneOf(HitResult.GetActor(), TagsToCollideWith))
			{
				// Then undo the application of the movement offset.
				SetActorLocation(OldActorLocation);
			}
		}
	}

	if (Value.X != 0 || Value.Y != 0)
	{
		// Get current rotation.
		auto ActorRotation = GetActorRotation();

		// Get target rotation.
		auto ActorLocation = GetActorLocation();
		FVector DeltaLocation2(Value.X, Value.Y, 0.0f);
		auto LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, ActorLocation + DeltaLocation2);

		// Lerp to target rotation.
		auto NewRotation = FMath::RInterpTo(ActorRotation, LookAtRotation, DeltaTime, RotationInterpSpeed);

		// Set rotation to interpolated rotation value.
		SetActorRotation(NewRotation);
	}
}
