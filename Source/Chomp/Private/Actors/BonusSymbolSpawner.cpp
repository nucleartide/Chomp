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
		SpawnBonusSymbol();
	}
	else if (DotsConsumed == SecondSymbolDotThreshold && !IsValid(SpawnedBonusSymbol))
	{
		SpawnBonusSymbol();
	}
}

UE5Coro::TCoroutine<> ABonusFruitSpawner::SpawnBonusSymbol()
{
	// Grab some variables.
	const auto World = FSafeGet::World(this);
	const auto LevelInstance = ULevelLoader::GetInstance(LevelLoader);
	const auto SpawnLocation = LevelInstance->GetBonusSymbolTile();
	const auto SpawnLocationWorld = LevelInstance->GridToWorld3D(SpawnLocation);

	// Bogus location.
	auto BogusSpawn = ULevelLoader::GetInstance(LevelLoader)->GridToWorld(FGridLocation{-100, -100});
	FVector BogusLocation(BogusSpawn.X, BogusSpawn.Y, 0.0f);

	// Spawn in a spot away from the player to avoid spawn failures.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	// Spawn in bogus location.
	SpawnedBonusSymbol = World->SpawnActor<ABonusSymbol>(
		BonusSymbolToSpawn,
		BogusLocation,
		FRotator::ZeroRotator,
		Params
	);
	check(SpawnedBonusSymbol);

	// Need this in order to change the location.
	SpawnedBonusSymbol->SetMobility(EComponentMobility::Movable);
	SpawnedBonusSymbol->SetActorLocation(SpawnLocationWorld);

	// Wait.
	co_await UE5Coro::Latent::Seconds(NumSecondsUntilSymbolRemoval);

	// Time limit reached without consumption, destroy automatically.
	if (IsValid(SpawnedBonusSymbol))
	{
		SpawnedBonusSymbol->Destroy();
	}
}
