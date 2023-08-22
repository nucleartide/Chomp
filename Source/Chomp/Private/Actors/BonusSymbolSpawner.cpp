#include "BonusSymbolSpawner.h"

#include "BonusSymbol.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/SafeGet.h"

void ABonusFruitSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Pre-conditions.
	check(BonusSymbolToSpawn);
	
	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	ChompGameState->OnDotsConsumedUpdated.AddUniqueDynamic(this, &ABonusFruitSpawner::HandleDotsConsumedChanged);
}

void ABonusFruitSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	ChompGameState->OnDotsConsumedUpdated.RemoveDynamic(this, &ABonusFruitSpawner::HandleDotsConsumedChanged);
}

void ABonusFruitSpawner::HandleDotsConsumedChanged(int DotsConsumed)
{
	if (DotsConsumed == FirstSymbolDotThreshold)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const auto World = FSafeGet::World(this);
		const auto LevelInstance = ULevelLoader::GetInstance(LevelLoader);
		const auto SpawnLocation = LevelInstance->GetBonusSymbolTile();
		const auto SpawnLocationWorld = LevelInstance->GridToWorld3D(SpawnLocation);
		
		const auto SpawnedSymbol = World->SpawnActor<ABonusSymbol>(
			BonusSymbolToSpawn,
			SpawnLocationWorld,
			FRotator::ZeroRotator,
			Params
		);
		check(SpawnedSymbol);
		
		// [x] TODO: configure all this in the scene
		// [x] TODO: when consumed, award points from configured field
		// [x] TODO: install editor debug symbols
	}
}
