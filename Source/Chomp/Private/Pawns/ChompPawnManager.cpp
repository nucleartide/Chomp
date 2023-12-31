#include "Pawns/ChompPawnManager.h"
#include "ChompGameState.h"
#include "Pawns/ChompPawn.h"
#include "Kismet/GameplayStatics.h"

void AChompPawnManager::BeginPlay()
{
	Super::BeginPlay();
	HandleGameRestarted(EChompGameStateEnum::None, EChompGameStateEnum::Playing);
}

void AChompPawnManager::HandleGameRestarted(EChompGameStateEnum OldState, EChompGameStateEnum NewState)
{
	if (NewState == EChompGameStateEnum::Playing)
	{
		auto LevelInstance = ULevelLoader::GetInstance(Level);
		auto SpawnWorldPosition2D = LevelInstance->GridToWorld(SpawnGridPosition);
		FVector SpawnWorldPosition{SpawnWorldPosition2D.X, SpawnWorldPosition2D.Y, 0.0f};

		if (ChompPawnInstance.IsValid())
		{
			ChompPawnInstance->SetActorLocation(SpawnWorldPosition);
			ChompPawnInstance->SetActorRotation(SpawnRotation);
		}
		else
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn actor.
			auto Actor = GetWorld()->SpawnActor<AChompPawn>(ChompPawn, SpawnWorldPosition, SpawnRotation, SpawnParams);
			check(Actor);

			// Save reference.
			ChompPawnInstance = Actor;

			// Also get the current player controller to possess this newly spawned pawn.
			auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			Controller->Possess(Actor);
		}
	}
}
