#include "ChompGameState.h"
#include "Utils/Debug.h"

void AChompGameState::ResetDots(int NumberOfDots)
{
    UpdateScore(0);
    UpdateNumberOfDotsRemaining(NumberOfDots);
}

void AChompGameState::ConsumeDot()
{
    UpdateScore(Score + SCORE_MULTIPLIER);
    UpdateNumberOfDotsRemaining(NumberOfDotsRemaining - 1);
}

void AChompGameState::UpdateScore(int NewScore)
{
    Score = NewScore;
    OnScoreUpdatedDelegate.Broadcast(NewScore);
}

void AChompGameState::UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining)
{
    NumberOfDotsRemaining = NewNumberOfDotsRemaining;
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
