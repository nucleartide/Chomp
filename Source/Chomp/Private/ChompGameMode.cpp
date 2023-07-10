// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChompGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/ChompPlayerController.h"
#include "Utils/Debug.h"

void AChompGameMode::SetGameState(PacmanGameState NewState)
{
    if (NewState == PacmanGameState::Playing)
    {
        OnGameRestartedDelegate.Broadcast();
    }

    // NOTE: THIS MUST COME AFTER THE RESTART, otherwise player will consume dots before level restarts.
    GameState = NewState;
    OnGameStateChangedDelegate.Broadcast(NewState);
}
