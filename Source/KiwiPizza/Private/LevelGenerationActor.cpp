// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGenerationActor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Debug.h"
#include "ConsumableDotActor.h"
#include "PacmanGameMode.h"

// Called when the game starts or when spawned
void ALevelGenerationActor::BeginPlay()
{
	Super::BeginPlay();
	RegenerateDots();

	//
	// Attach event handler.
	//

	// Fetch GameMode. This couples this class tightly to the game mode, but that's okay.
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	// Cast to correct class.
	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	// Attach event handler.
	PacmanGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::RegenerateDots);
}

void ALevelGenerationActor::RegenerateDots()
{
	//
	// Load level.
	//

	auto DefaultObject = LevelLoader.GetDefaultObject();
	check(DefaultObject);

	auto Level = Cast<ULevelLoader>(DefaultObject);
	check(Level);

	Level->LoadLevel();

	DEBUG_LOG(TEXT("Level width: %d"), Level->GetLevelWidth());
	DEBUG_LOG(TEXT("Level height: %d"), Level->GetLevelHeight());

	for (int32 x = 0; x < Level->StringList.Num(); x++)
	{
		auto Element = Level->StringList[x];
		auto Len = Element.Len();

		for (int32 y = 0; y < Len; y++)
		{
			auto Character = Element[y];
			if (Character == 'W' || Character == '|')
			{
				auto World = GetWorld();
				check(World);

				// Define the spawn parameters.
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Set the desired location.
				auto LocationX = x * 100.0f - Level->StringList.Num() * 0.5f * 100.0f + 50.0f;
				auto LocationY = y * 100.0f - Element.Len() * 0.5f * 100.0f + 50.0f;
				FVector Location(LocationY, LocationX, 0.0f);

				// Select tile.
				TSubclassOf<AStaticMeshActor> SelectedTile;
				if (Character == 'W')
					SelectedTile = WallTile;
				else if (Character == '|')
					SelectedTile = GateTile;

				// Spawn the actor at the desired location.
				auto SpawnedActor = World->SpawnActor<AStaticMeshActor>(SelectedTile, Location, FRotator::ZeroRotator, SpawnParams);
				check(SpawnedActor);
			}
			else if (Character == ' ')
			{
				auto World = GetWorld();
				check(World);

				// Set spawn params.
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Set the desired location for the actor
				auto LocationX = x * 100.0f - Level->StringList.Num() * 0.5f * 100.0f + 50.0f;
				auto LocationY = y * 100.0f - Element.Len() * 0.5f * 100.0f + 50.0f;
				FVector Location(LocationY, LocationX, 0.0f);

				// Spawn the actor at the desired location
				FVector BogusLocation(10000.0f, 10000.0f, 10000.0f);
				auto Actor = World->SpawnActor<AStaticMeshActor>(PacmanDot, BogusLocation, FRotator::ZeroRotator, Params); // Spawn in a spot away from the player to avoid spawn failures.
				check(Actor);

				// Once successful, set the actor location once again.
				Actor->SetMobility(EComponentMobility::Movable);
				Actor->SetActorLocation(Location);

				// Cast.
				auto ConsumableDotActor = Cast<AConsumableDotActor>(Actor);
				check(ConsumableDotActor);

				// Attach handler to when ConsumableDotActor is destroyed.
				ConsumableDotActor->OnDotConsumedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::HandleDotConsumption);

				// Bump counter.
				NumDotsGenerated++;
			}
		}
	}

	DEBUG_LOG(TEXT("%d dots generated."), NumDotsGenerated);
}

void ALevelGenerationActor::HandleDotConsumption()
{
	NumDotsGenerated--;
	DEBUG_LOG(TEXT("Dot consumed, remaining dots: %d"), NumDotsGenerated);

	if (NumDotsGenerated == 0)
	{
		OnLevelClearedDelegate.Broadcast();

		// Fetch the game mode.
		auto GameMode = GetWorld()->GetAuthGameMode();
		check(GameMode);

		auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
		check(PacmanGameMode);

		PacmanGameMode->SetGameState(PacmanGameState::GameOverWin);
	}
}
