// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelGenerationActor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Debug.h"
#include "PacmanDotActor.h"

// Called when the game starts or when spawned
void ALevelGenerationActor::BeginPlay()
{
	Super::BeginPlay();

	//
	// Attach event handler.
	//

	/*
		// Fetch GameMode.
		auto GameMode = GetWorld()->GetAuthGameMode();
		check(GameMode);

		// Cast to correct class.
		auto PacmanGameMode = Cast<AKiwiPizzaGameModeBase>(GameMode);
		check(PacmanGameMode);

		// Attach event handler.
		PacmanGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::RegenerateDots);
	*/

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
			if (Character == 'W')
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

				// Spawn the actor at the desired location.
				auto SpawnedActor = World->SpawnActor<AStaticMeshActor>(WallTile, Location, FRotator::ZeroRotator, SpawnParams);
				check(SpawnedActor);

				// Set the tags of the actor.
				// auto Tags = WallTile.GetDefaultObject()->Tags;
				// for (auto Tag : Tags)
					// SpawnedActor->Tags.Add(Tag);

				// Set the static mesh of the actor
				// auto StaticMesh = WallTile.GetDefaultObject()->GetStaticMeshComponent()->GetStaticMesh();
				// SpawnedActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				// SpawnedActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
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
				auto Actor = World->SpawnActor<AStaticMeshActor>(PacmanDot, Location, FRotator::ZeroRotator, Params);
				check(Actor);

				// Cast.
				auto PacmanDotActor = Cast<APacmanDotActor>(Actor);
				check(PacmanDotActor);

				// Attach handler to when PacmanDotActor is destroyed.
				PacmanDotActor->OnDotConsumedDelegate.AddUniqueDynamic(this, &ALevelGenerationActor::HandleDotConsumption);

				// Bump counter.
				NumDotsGenerated++;
			}
		}
	}

	DEBUG_LOG(TEXT("%d dots generated."), NumDotsGenerated);
}

/*
// TODO: Save this for later.
void ALevelGenerationActor::ResetPlayerPosition()
{
	auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	auto Pawn = PlayerController->GetPawn();
	auto StartingLocation = PlayerStartingLocation->GetActorLocation();
	Pawn->SetActorLocation(StartingLocation);
}
*/

void ALevelGenerationActor::RegenerateDots()
{
	// TODO: Gonna need this.
}

void ALevelGenerationActor::HandleDotConsumption()
{
	NumDotsGenerated--;
	DEBUG_LOG(TEXT("Dot consumed, remaining dots: %d"), NumDotsGenerated);

	if (NumDotsGenerated == 0)
	{
		OnLevelClearedDelegate.Broadcast();
	}
}
