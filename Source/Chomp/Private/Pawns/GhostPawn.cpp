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
		GhostController->OnGhostStateChanged.AddUniqueDynamic(this, &AGhostPawn::HandleGhostStateChanged);
	}
}

void AGhostPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (const auto GhostController = GetController<AGhostAiController>())
		GhostController->OnGhostStateChanged.RemoveDynamic(this, &AGhostPawn::HandleGhostStateChanged);
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	if (ChompGameState->GetEnum() != EChompGameStateEnum::Playing)
	{
		return;
	}

	const auto GhostController = GetController<AGhostAiController>();
	check(GhostController);
	
	if (ChompGameState->GetSubstateEnum() == EChompPlayingSubstateEnum::Frightened &&
		!GhostController->GetHasBeenEaten() &&
		Cast<AChompPawn>(Other))
	{
		GhostController->Consume();
	}
}

void AGhostPawn::HandleGhostStateChanged(const EGhostState NewGhostState)
{
	const auto IsVisible = NewGhostState != EGhostState::Eaten;
	HeadComponentRef->SetVisibility(IsVisible);
	BodyComponentRef->SetVisibility(IsVisible);
	
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
		// No-op. Can't see anything anyway.
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
