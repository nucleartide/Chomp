#include "Pawns/GhostPawn.h"
#include "ChompGameState.h"
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
		const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
		ChompGameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(this, &AGhostPawn::HandlePlayingSubstateChanged);
	}
}

void AGhostPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	{
		const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
		ChompGameState->OnGamePlayingStateChangedDelegate.RemoveDynamic(this, &AGhostPawn::HandlePlayingSubstateChanged);
	}
}

void AGhostPawn::HandlePlayingSubstateChanged(
	EChompPlayingSubstateEnum OldSubstate,
	EChompPlayingSubstateEnum NewSubstate)
{
	// Pre-conditions.
	check(OldSubstate != NewSubstate);

	if (NewSubstate == EChompPlayingSubstateEnum::Frightened)
	{
		HeadComponentRef->SetMaterial(0, FrightenedMaterial);
		BodyComponentRef->SetMaterial(0, FrightenedMaterial);
	}
	else
	{
		HeadComponentRef->SetMaterial(0, NotFrightenedMaterial);
		BodyComponentRef->SetMaterial(0, NotFrightenedMaterial);
	}
}

FGridLocation AGhostPawn::GetStartingPosition() const
{
	return StartingPosition;
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
