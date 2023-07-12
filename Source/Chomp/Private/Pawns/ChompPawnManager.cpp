#include "Pawns/ChompPawnManager.h"
#include "ChompGameMode.h"
#include "ChompGameState.h"
#include "Pawns/ChompPawn.h"

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
		ChompPawn->SetActorLocation(GetActorLocation());
		ChompPawn->SetActorRotation(GetActorRotation());
	}
}
