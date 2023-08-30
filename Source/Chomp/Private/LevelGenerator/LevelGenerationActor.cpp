#include "LevelGenerator/LevelGenerationActor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Actors/ConsumableDotActor.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"
#include "LevelDataAsset.h"
#include "Actors/ConsumableEnergizerActor.h"
#include "Controllers/ChompPlayerController.h"
#include "Controllers/GhostAiController.h"
#include "GameState/ChompSaveGame.h"
#include "GameState/LocalStorageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/ChompPawnManager.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

void ALevelGenerationActor::BeginPlay()
{
	Super::BeginPlay();

	// Once level data is loaded (in PostInitializeComponents), generate tiles based off tile data.
	GenerateTiles();

	// Lastly, add a listener to regenerate tiles when the game restarts.
	const auto ChompGameMode = GetWorld()->GetGameState<AChompGameState>();
	ChompGameMode->OnGameStateChanged.AddUniqueDynamic(this, &ALevelGenerationActor::ResetStateOfEverything);
}

void ALevelGenerationActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Load level data before doing anything else.
	const auto Level = ULevelLoader::GetInstance(LevelLoader);
	Level->LoadLevel();
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
				auto SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(
					SelectedTile, Location, FRotator::ZeroRotator, SpawnParams);
				check(SpawnedActor);

				// Keep reference to actor so we can destroy it later on.
				Tiles.Add(SpawnedActor);
			}
			else if (Character == ' ' || Character == 'S' || Character == 'B')
			{
				// Spawn dot actor.
				auto BogusSpawn = Level->GridToWorld(FGridLocation{-100, -100});
				FVector BogusLocation(BogusSpawn.X, BogusSpawn.Y, 0.0f);

				// Spawn in a spot away from the player to avoid spawn failures.
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				auto ConsumableDotActor = GetWorld()->SpawnActor<AConsumableDotActor>(
					PacmanDot,
					BogusLocation,
					FRotator::ZeroRotator,
					Params
				);
				check(ConsumableDotActor);

				// Need this so we can change the dot's location.
				ConsumableDotActor->SetMobility(EComponentMobility::Movable);

				// Set the actor location to the actual location.
				FGridLocation GridPosition{X, Y};
				auto WorldPosition = Level->GridToWorld(GridPosition);
				FVector Location(WorldPosition.X, WorldPosition.Y, 0.0f);
				ConsumableDotActor->SetActorLocation(Location);

				// Keep track of the generated dot.
				Tiles.Add(ConsumableDotActor);
				NumberOfDotsRemaining++;
			}
			else if (Character == 'O')
			{
				// Compute spawn position.
				const auto WorldPosition = [X, Y, Level]()
				{
					const FGridLocation GridPosition{X, Y};
					const auto WorldPosition2D = Level->GridToWorld(GridPosition);
					return FVector(WorldPosition2D.X, WorldPosition2D.Y, 0.0);
				}();

				// Compute spawn parameters.
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Spawn and check.
				const auto Actor = GetWorld()->SpawnActor<AConsumableEnergizerActor>(
					EnergizerDot,
					WorldPosition,
					FRotator::ZeroRotator,
					Params
				);
				check(Actor);

				// Keep track of the generated actor for later cleanup.
				Tiles.Add(Actor);
			}
			else if (Character == 'x' || Character == 'o' || Character == 'G' || Character == 'g')
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

void ALevelGenerationActor::ResetStateOfEverything(const EChompGameStateEnum OldState,
                                                   const EChompGameStateEnum NewState)
{
	// Pre-conditions.
	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	const auto OldNumberOfDotsConsumed = ChompGameState->GetNumberOfDotsConsumed();
	check(CurrentLevel);

	// We've reached this level, at least!
	GetGameInstance()->GetSubsystem<ULocalStorageSubsystem>()->GetSaveGame()->SetHighScoreLevel(CurrentLevel);

	if (OldState != EChompGameStateEnum::LostLife && NewState == EChompGameStateEnum::Playing)
	{
		// First, clean up the map.
		ClearLeftoverTiles();

		// Then, reset the ghosts.
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostAiController::StaticClass(), FoundActors);
		for (const auto& Actor : FoundActors)
		{
			const auto Controller = Cast<AGhostAiController>(Actor);
			check(Controller);
			Controller->HandleGameStateChanged(OldState, NewState);
		}

		// Then, reset the player pawn position.
		const auto ChompPawnManager = Cast<AChompPawnManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AChompPawnManager::StaticClass())
		);
		check(ChompPawnManager);
		ChompPawnManager->HandleGameRestarted(OldState, NewState);

		// Then, reset the movement brain.
		const auto PlayerController = FSafeGet::PlayerController(this, 0);
		if (const auto ChompPlayerController = Cast<AChompPlayerController>(PlayerController))
			ChompPlayerController->HandleGameRestarted(OldState, NewState);

		// Then generate tiles.
		GenerateTiles();

		// Note that this particular ordering ensures that no overlap conditions are triggered.
	}
	else if (OldState == EChompGameStateEnum::LostLife && NewState == EChompGameStateEnum::Playing)
	{
		// Reset the ghosts.
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostAiController::StaticClass(), FoundActors);
		for (const auto& Actor : FoundActors)
		{
			const auto Controller = Cast<AGhostAiController>(Actor);
			check(Controller);
			Controller->HandleGameStateChanged(OldState, NewState);
		}

		// Reset the player pawn position.
		const auto ChompPawnManager = Cast<AChompPawnManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AChompPawnManager::StaticClass())
		);
		check(ChompPawnManager);
		ChompPawnManager->HandleGameRestarted(OldState, NewState);

		// Reset the movement brain.
		const auto PlayerController = FSafeGet::PlayerController(this, 0);
		if (const auto ChompPlayerController = Cast<AChompPlayerController>(PlayerController))
			ChompPlayerController->HandleGameRestarted(OldState, NewState);

		// Note that this particular ordering ensures that no overlap conditions are triggered.
	}

	// Post-conditions.
	checkf(
		OldState == EChompGameStateEnum::LostLife && NewState == EChompGameStateEnum::Playing
		? OldNumberOfDotsConsumed.GetValue() == ChompGameState->GetNumberOfDotsConsumed().GetValue()
		: true,
		TEXT("Dots should not reset upon losing a life.")
	);
}
