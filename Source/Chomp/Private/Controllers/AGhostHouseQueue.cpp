#include "AGhostHouseQueue.h"

#include "Utils/SafeGet.h"

AGhostHouseQueue::AGhostHouseQueue(): AActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGhostHouseQueue::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const auto World = FSafeGet::World(this);
	const auto Time = World->GetTimeSeconds();
	const auto GameState = World->GetGameState<AChompGameState>();
	if (Time > GameState->GetNumberOfDotsConsumed().GetLastUpdatedTime() + TimeToForceGhostOutOfHouse)
	{
		Things.Pop();
		GameState->UpdateNumberOfDotsConsumed(GameState->GetNumberOfDotsConsumed().GetValue());
	}
}
