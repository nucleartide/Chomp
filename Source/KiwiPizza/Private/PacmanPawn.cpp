#include "PacmanPawn.h"
#include "PacmanGameMode.h"
#include "Debug.h"
#include "GhostPawn.h"

APacmanPawn::APacmanPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APacmanPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WrapAroundWorld();
}

void APacmanPawn::MoveVector(FVector2D Value)
{
	// Declare some variables.
	FVector DeltaLocation(Value.Y, Value.X, 0.0f);
	auto ActorScale = GetActorScale3D();
	float SphereDiameter = 100.0f * ActorScale.X;
	float SphereRadius = SphereDiameter * 0.5f * 0.5f; // Halve the radius a second time for a smaller collision sphere.
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	//
	// Slide along horizontal walls
	//

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
				if (HitResult.GetActor()->ActorHasTag(FName("LevelGeometry")))
				{
					DeltaLocation.X = 0;
				}
			}
		}
	}

	//
	// Slide along vertical walls
	//

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
				if (HitResult.GetActor()->ActorHasTag(FName("LevelGeometry")))
				{
					DeltaLocation.Y = 0;
				}
			}
		}
	}

	// Let's apply the offset for now.
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
				if (HitActor->ActorHasTag(FName("LevelGeometry")))
				{
					// Then undo the application of the movement offset.
					SetActorLocation(OldActorLocation);
				}
			}
		}
	}
}

void APacmanPawn::WrapAroundWorld()
{
	// Declare some variables.
	auto LevelHeight = Level.GetDefaultObject()->GetLevelHeight();
	auto LevelWidth = Level.GetDefaultObject()->GetLevelWidth();
	auto Location = GetActorLocation();
	bool IsWithinBounds = true;

	// Check X axis.
	auto HalfHeight = LevelHeight * 0.5f * 100.0f;
	if (Location.X < -HalfHeight || Location.X > HalfHeight)
	{
		// Update state.
		IsWithinBounds = false;

		// Update X component of Location.
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
	}

	// Check Y axis.
	auto HalfWidth = LevelWidth * 0.5f * 100.0f;
	if (Location.Y < -HalfWidth || Location.Y > HalfWidth)
	{
		// Update state.
		IsWithinBounds = false;

		// Update Y component of Location.
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
	}

	// Update actor location.
	SetActorLocation(Location);
}

void APacmanPawn::NotifyActorBeginOverlap(AActor *Other)
{

	//
	// Early return if we aren't in a "Playing" state.
	//

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	if (PacmanGameMode->GameState != PacmanGameState::Playing)
		return;

	// If we overlapped with a dot, then consume the other dot.
	if (Other->Tags.Contains(FName("SmallDot"))) // TODO: One could check tags or cast against a type. Checking tags is error prone, but casting has dependency issues. Wonder which is the better tradeoff?
	{
		Other->Destroy();
	}
	// Otherwise, if we overlapped with a ghost, then log for now.
	else if (Cast<AGhostPawn>(Other))
	{
		DEBUG_LOG(TEXT("Overlapped with ghost pawn. Pawn name: %s"), *Other->GetHumanReadableName());
		OnPacmanDiedDelegate.Broadcast();
	}
}

void APacmanPawn::NotifyActorEndOverlap(AActor *Other)
{
	// No-op.
}
