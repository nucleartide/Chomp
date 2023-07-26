#include "LevelGenerator/LevelGenerationActor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Actors/ConsumableDotActor.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"
#include "Utils/Debug.h"

void ALevelGenerationActor::BeginPlay()
{
	Super::BeginPlay();

	// Load level data before doing anything else.
	const auto Level = ULevelLoader::GetInstance(LevelLoader);
	Level->LoadLevel();

	// Once level data is loaded, generate tiles based off tile data.
	GenerateTiles();

	// Lastly, add a listener to regenerate tiles when the game restarts.
	const auto ChompGameMode = GetWorld()->GetGameState<AChompGameState>();
	ChompGameMode->OnLateGameStateChangedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::ResetTiles);
}

void ALevelGenerationActor::ClearLeftoverTiles()
{
	DEBUG_LOG(TEXT("Destroying all leftover tiles..."));

	for (const auto Tile : Tiles)
	{
		if (IsValid(Tile))
		{
			Tile->Destroy();
		}
	}

	Tiles.Empty();
}

void ALevelGenerationActor::GenerateTiles()
{
	auto Level = ULevelLoader::GetInstance(LevelLoader);
	auto NumberOfDotsRemaining = 0;

	for (int X = 0; X < Level->GetLevelHeight(); X++)
	{
		auto Row = Level->StringList[X];

		for (int Y = 0; Y < Level->GetLevelWidth(); Y++)
		{
			if (auto Character = Row[Y]; Character == 'W' || Character == '-')
			{
				// Compute selected tile.
				TSubclassOf<AStaticMeshActor> SelectedTile;
				if (Character == 'W')
					SelectedTile = WallTile;
				else if (Character == '-')
					SelectedTile = GateTile;
				else
					check(false);

				// Compute location.
				FGridLocation GridPosition{X, Y};
				auto WorldPosition = Level->GridToWorld(GridPosition);
				FVector Location(WorldPosition.X, WorldPosition.Y, 0.0f);

				// Compute spawn params.
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Spawn the actor at the desired location.
				auto SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(SelectedTile, Location, FRotator::ZeroRotator, SpawnParams);
				check(SpawnedActor);

				// Keep reference to actor so we can destroy it later on.
				Tiles.Add(SpawnedActor);
			}
			else if (Character == ' ')
			{
				// Spawn dot actor.
				auto BogusSpawn = Level->GridToWorld(FGridLocation{-100, -100});
				FVector BogusLocation(BogusSpawn.X, BogusSpawn.Y, 0.0f); // Spawn in a spot away from the player to avoid spawn failures.
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				auto Actor = GetWorld()->SpawnActor<AStaticMeshActor>(PacmanDot, BogusLocation, FRotator::ZeroRotator, Params); 
				check(Actor);

				// Need this so we can change the dot's location.
				Actor->SetMobility(EComponentMobility::Movable);

				// Set the actor location to the actual location.
				FGridLocation GridPosition{X, Y};
				auto WorldPosition = Level->GridToWorld(GridPosition);
				FVector Location(WorldPosition.X, WorldPosition.Y, 0.0f);
				Actor->SetActorLocation(Location);

				// Finally, attach a handler for when a dot is consumed.
				auto ConsumableDotActor = Cast<AConsumableDotActor>(Actor);
				check(ConsumableDotActor);

				// Keep track of the generated dot.
				Tiles.Add(ConsumableDotActor);
				NumberOfDotsRemaining++;
			}
			else if (Character == 'x' || Character == 'o')
			{
				// No-op.
			}
			else
			{
				check(false);
			}
		}
	}

	GetWorld()->GetGameState<AChompGameState>()->ResetDots(NumberOfDotsRemaining);
}

void ALevelGenerationActor::ResetTiles(const EChompGameState OldState, const EChompGameState NewState)
{
	if (NewState == EChompGameState::Playing)
	{
		ClearLeftoverTiles();
		GenerateTiles();
	}
}
