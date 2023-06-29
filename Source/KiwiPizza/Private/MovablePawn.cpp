#include "MovablePawn.h"
#include "LevelLoader.h"
#include "GameplayTag.h"

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

void AMovablePawn::MoveVector(FVector2D Value)
{
	// Declare some variables.
	FVector DeltaLocation(Value.Y, Value.X, 0.0f);
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
			if (HitResult.bBlockingHit)
			{
				if (HitResult.GetActor()->ActorHasTag(GameplayTag::LevelGeometry))
				{
					DeltaLocation.X = 0;
				}
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
			if (HitResult.bBlockingHit)
			{
				if (HitResult.GetActor()->ActorHasTag(GameplayTag::LevelGeometry))
				{
					DeltaLocation.Y = 0;
				}
			}
		}
	}

	// Let's apply the offset first.
	auto OldActorLocation = GetActorLocation();
	AddActorLocalOffset(DeltaLocation, false);

	// However, in the case where we're overlapping with a wall after applying the offset,
	if (DeltaLocation.X != 0.0f || DeltaLocation.Y != 0.0f)
	{
		// Perform overlap check.
		TArray<FOverlapResult> HitResults;
		GetWorld()->OverlapMultiByChannel(HitResults, GetActorLocation(), FQuat::Identity, ECC_Visibility, SphereShape);

		for (auto HitResult : HitResults)
		{
			// If there was an overlap,
			if (HitResult.bBlockingHit)
			{
				// Then check the Actor.
				auto HitActor = HitResult.GetActor();

				// If the Actor is a wall,
				if (HitActor->ActorHasTag(GameplayTag::LevelGeometry))
				{
					// Then undo the application of the movement offset.
					SetActorLocation(OldActorLocation);
				}
			}
		}
	}
}
