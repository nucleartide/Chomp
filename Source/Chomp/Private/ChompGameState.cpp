#include "ChompGameState.h"
#include "Utils/Debug.h"

void AChompGameState::ResetDots(int NumberOfDots)
{
	Score = 0;
    OnScoreUpdatedDelegate.Broadcast(Score);
	NumberOfDotsRemaining = NumberOfDots;
}

void AChompGameState::ConsumeDot()
{
    Score += 1 * SCORE_MULTIPLIER;
    OnScoreUpdatedDelegate.Broadcast(Score);

    NumberOfDotsRemaining--;
    if (NumberOfDotsRemaining == 0)
    {
        OnDotsClearedDelegate.Broadcast();
        TransitionTo(EChompGameState::GameOverWin);
    }
}

void AChompGameState::NotifyPlayerDeath()
{
    TransitionTo(EChompGameState::GameOverLose);
}

void AChompGameState::TransitionTo(EChompGameState NewState)
{
    auto OldState = GameState;
    check(OldState != NewState);

    GameState = NewState;
    OnGameStateChangedDelegate.Broadcast(OldState, NewState);
    OnLateGameStateChangedDelegate.Broadcast(OldState, NewState);
    DEBUG_LOG(TEXT("Transitioned from %d to %d"), OldState, NewState);
}

EChompGameState AChompGameState::GetEnum()
{
    return GameState;
}

int AChompGameState::GetScore()
{
    return Score;
}
