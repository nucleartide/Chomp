#include "GhostHouseQueue.h"

#include "Utils/SafeGet.h"

AGhostHouseQueue::AGhostHouseQueue(): AActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGhostHouseQueue::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const auto World = FSafeGet::World(this);
	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
	const auto TimeSeconds = World->GetTimeSeconds();

	if (
		const auto NumberOfDotsConsumed = GameState->GetNumberOfDotsConsumed();
		TimeSeconds > NumberOfDotsConsumed.GetLastUpdatedTime() + TimeToForceGhostOutOfHouse)
	{
		Things.Pop();
		GameState->UpdateNumberOfDotsConsumed(NumberOfDotsConsumed.GetValue());
	}
}
