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
	// Pre-conditions.
	check(FirstSymbolDotThreshold < SecondSymbolDotThreshold);
	
	if (DotsConsumed == FirstSymbolDotThreshold)
	{
		SpawnedBonusSymbol = SpawnBonusSymbol();

		// Post-conditions.
		check(IsValid(SpawnedBonusSymbol));
	}
	else if (DotsConsumed == SecondSymbolDotThreshold && !IsValid(SpawnedBonusSymbol))
	{
		SpawnedBonusSymbol = SpawnBonusSymbol();

		// Post-conditions.
		check(IsValid(SpawnedBonusSymbol));
	}
}

ABonusSymbol* ABonusFruitSpawner::SpawnBonusSymbol() const
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const auto World = FSafeGet::World(this);
	const auto LevelInstance = ULevelLoader::GetInstance(LevelLoader);
	const auto SpawnLocation = LevelInstance->GetBonusSymbolTile();
	const auto SpawnLocationWorld = LevelInstance->GridToWorld3D(SpawnLocation);

	const auto BonusSymbol = World->SpawnActor<ABonusSymbol>(
		BonusSymbolToSpawn,
		SpawnLocationWorld,
		FRotator::ZeroRotator,
		Params
	);

	// Post-conditions.
	check(BonusSymbol);
	
	return BonusSymbol;
}
