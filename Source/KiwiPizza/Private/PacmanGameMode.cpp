// Copyright Epic Games, Inc. All Rights Reserved.

#include "PacmanGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanPlayerController.h"
#include "Debug.h"

/*
// TODO: On depletion of all dots, invoke the OnDotsDepleted event
// TODO: this will alert the PacmanGameMode (which subscribes to the Levelgenerationactor)
// TODO: the pacman game mode will then emit a "game over win" event
void APacmanGameMode::Tick(float DeltaTime)
{
    if (FoundActors.Num() == 0)
    {
        // TODO: Update state on the GameModeBase.
        if (GameState != GameOverWin)
        {
            GameState = GameOverWin;
            OnPacmanGameStateChangedDelegate.Broadcast(GameOverWin);
        }
    }
}
*/
