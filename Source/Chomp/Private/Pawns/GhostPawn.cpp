#include "Pawns/GhostPawn.h"
#include "ChompGameState.h"
#include "ChompPawn.h"
#include "Controllers/GhostAiController.h"
#include "Utils/SafeGet.h"

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	{
		const auto HeadComponents = GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("Head"));
		check(HeadComponents.Num() == 1);
		HeadComponentRef = Cast<UStaticMeshComponent>(HeadComponents[0]);
		check(HeadComponentRef);
	}

	{
		auto BodyComponents = GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("Body"));
		check(BodyComponents.Num() == 1);
		BodyComponentRef = Cast<UStaticMeshComponent>(BodyComponents[0]);
		check(BodyComponentRef);
	}

	check(GhostHouseQueue);
	check(NotFrightenedMaterial);
	check(FrightenedMaterial);

	{
		const auto GhostController = GetController<AGhostAiController>();
		check(GhostController);
		GhostController->OnGhostStateChanged.AddUniqueDynamic(this, &AGhostPawn::UpdateVisibility);
		GhostController->OnGhostStateChanged.AddUniqueDynamic(this, &AGhostPawn::UpdateMaterial);
	}
}

void AGhostPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (const auto GhostController = GetController<AGhostAiController>())
	{
		GhostController->OnGhostStateChanged.RemoveDynamic(this, &AGhostPawn::UpdateVisibility);
		GhostController->OnGhostStateChanged.RemoveDynamic(this, &AGhostPawn::UpdateMaterial);
	}
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	if (!ChompGameState->IsPlaying())
		return;

	const auto GhostController = GetController<AGhostAiController>();
	check(GhostController);

	if (const auto IsPlayer = Cast<AChompPawn>(Other);
		IsPlayer &&
		ChompGameState->IsFrightened() &&
		!GhostController->IsEaten())
	{
		GhostController->Consume();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AGhostPawn::UpdateVisibility(const EGhostState NewGhostState)
{
	const auto IsVisible = NewGhostState != EGhostState::Eaten;
	HeadComponentRef->SetVisibility(IsVisible);
	BodyComponentRef->SetVisibility(IsVisible);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AGhostPawn::UpdateMaterial(const EGhostState NewGhostState)
{
	if (NewGhostState == EGhostState::Frightened)
	{
		HeadComponentRef->SetMaterial(0, FrightenedMaterial);
		BodyComponentRef->SetMaterial(0, FrightenedMaterial);
	}
	else if (NewGhostState == EGhostState::Normal)
	{
		HeadComponentRef->SetMaterial(0, NotFrightenedMaterial);
		BodyComponentRef->SetMaterial(0, NotFrightenedMaterial);
	}
	else if (NewGhostState == EGhostState::Eaten)
	{
		// No-op. Shouldn't be able to see anything.
	}
	else
	{
		checkf(false, TEXT("GhostState is not handled: %d"), NewGhostState)
	}
}

FGridLocation AGhostPawn::GetStartingPosition() const
{
	return StartingPosition;
}

FGridLocation AGhostPawn::GetGhostHouseReturnLocation() const
{
	return GhostHouseReturnLocation;
}

int AGhostPawn::GetDotsConsumedMovementThreshold() const
{
	return DotsConsumedMovementThreshold;
}

FGridLocation AGhostPawn::GetScatterOrigin() const
{
	return ScatterOrigin;
}

FGridLocation AGhostPawn::GetScatterDestination() const
{
	return ScatterDestination;
}

int AGhostPawn::GetLeaveGhostHousePriority() const
{
	return LeaveGhostHousePriority;
}

AGhostHouseQueue* AGhostPawn::GetGhostHouseQueue() const
{
	return GhostHouseQueue;
}

FLinearColor AGhostPawn::GetDebugColor() const
{
	return DebugColor;
}

double AGhostPawn::GetFrightenedMovementSpeed() const
{
	return FrightenedMovementSpeed;
}

double AGhostPawn::GetReturnToGhostHouseMovementSpeed() const
{
	return ReturnToGhostHouseMovementSpeed;
}
