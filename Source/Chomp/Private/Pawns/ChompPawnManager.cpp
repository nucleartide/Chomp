#include "Pawns/ChompPawnManager.h"
#include "ChompGameMode.h"
#include "ChompGameState.h"
#include "Pawns/ChompPawn.h"
#include "Kismet/GameplayStatics.h"

void AChompPawnManager::BeginPlay()
{
	Super::BeginPlay();
	HandleGameRestarted(EChompGameState::None, EChompGameState::Playing);
	GetWorld()->GetGameState<AChompGameState>()->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AChompPawnManager::HandleGameRestarted);
}

void AChompPawnManager::HandleGameRestarted(EChompGameState OldState, EChompGameState NewState)
{
	if (NewState == EChompGameState::Playing)
	{
		if (ChompPawnInstance.IsValid())
		{
			ChompPawnInstance->SetActorLocation(GetActorLocation());
			ChompPawnInstance->SetActorRotation(GetActorRotation());
		}
		else
		{
			auto SpawnLocation = GetActorLocation();
			auto SpawnRotation = GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn actor.
			auto Actor = GetWorld()->SpawnActor<AChompPawn>(ChompPawn, SpawnLocation, SpawnRotation, SpawnParams);
			check(Actor);

			// Save reference.
			ChompPawnInstance = Actor;

			// Also get the current player controller to possess this newly spawned pawn.
			auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			Controller->Possess(Actor);
		}
	}
}
