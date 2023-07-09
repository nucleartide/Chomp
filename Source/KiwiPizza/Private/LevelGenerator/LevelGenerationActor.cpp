#include "LevelGenerator/LevelGenerationActor.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

#include "Utils/Debug.h"
#include "Actors/ConsumableDotActor.h"
#include "PacmanGameMode.h"
#include "AStar/GridLocation.h"
#include "AStar/AStar.h"

void ALevelGenerationActor::BeginPlay()
{
	Super::BeginPlay();

	// Load level data before doing anything else.
	auto Level = ULevelLoader::GetInstance(LevelLoader);
	Level->LoadLevel();

	// Once level data is loaded, generate tiles based off tile data.
	GenerateTiles();

	// Lastly, add a listener to regenerate tiles when the game restarts.
	{
		auto GameMode = GetWorld()->GetAuthGameMode();
		auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
		check(PacmanGameMode);
		PacmanGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::ResetTiles);
	}
}

void ALevelGenerationActor::ClearLeftoverTiles()
{
	DEBUG_LOG(TEXT("Destroying all leftover tiles..."));

	for (auto Tile : Tiles)
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

	for (int X = 0; X < Level->GetLevelHeight(); X++)
	{
		auto Row = Level->StringList[X];

		for (int Y = 0; Y < Level->GetLevelWidth(); Y++)
		{
			auto Character = Row[Y];
			if (Character == 'W' || Character == '-')
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
				ConsumableDotActor->OnDotConsumedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::HandleDotConsumption);

				// Keep track of the generated dot.
				Tiles.Add(ConsumableDotActor);
				NumberOfDotsRemaining++;
			}
			else if (Character == 'x')
			{
				// No-op.
			}
			else
			{
				check(false);
			}
		}
	}
}

void ALevelGenerationActor::ResetTiles()
{
	ClearLeftoverTiles();
	GenerateTiles();
}

void ALevelGenerationActor::HandleDotConsumption()
{
	NumberOfDotsRemaining--;

	DEBUG_LOG(TEXT("Dot consumed. Remaining dots: %d"), NumberOfDotsRemaining);

	if (NumberOfDotsRemaining == 0)
	{
		OnLevelClearedDelegate.Broadcast();

		// Update the GameMode's state.
		//
		// We are already coupled to the PacmanGameMode because we need to listen to game restarts,
		// so reaching out to update the PacmanGameMode directly doesn't cause further harm.
		auto GameMode = GetWorld()->GetAuthGameMode();
		auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
		check(PacmanGameMode);
		PacmanGameMode->SetGameState(PacmanGameState::GameOverWin);
	}
}
