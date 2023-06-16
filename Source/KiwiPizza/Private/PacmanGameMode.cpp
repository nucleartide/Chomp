// Copyright Epic Games, Inc. All Rights Reserved.

#include "PacmanGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanPlayerController.h"
#include "Debug.h"

void APacmanGameMode::SetGameState(PacmanGameState NewState)
{
    if (NewState == PacmanGameState::Playing)
    {
        OnGameRestartedDelegate.Broadcast();
    }

    // NOTE: THIS MUST COME AFTER THE RESTART, otherwise player will consume dots before level restarts.
    GameState = NewState;
    OnGameStateChangedDelegate.Broadcast(NewState);
}
