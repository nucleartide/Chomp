#include "ChompGameState.h"
#include "Utils/Debug.h"

// Meant for debugging current wave:
AChompGameState::AChompGameState()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompGameState::ResetDots(int NumberOfDots)
{
    UpdateScore(0);
    UpdateNumberOfDotsRemaining(NumberOfDots);
}

void AChompGameState::ConsumeDot()
{
    UpdateScore(Score + ScoreMultiplier);
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

EChompGamePlayingState AChompGameState::GetCurrentWave()
{
    auto TimeSinceStart = GetWorld()->GetTimeSeconds();
    auto DurationCounter = 0.0;

    for (auto &Wave : Waves)
    {
        if (Wave.Duration < 0.0f)
        {
            auto DurationStart = DurationCounter;
            return TimeSinceStart >= DurationStart ? Wave.PlayingState : EChompGamePlayingState::None;
        }

        auto DurationStart = DurationCounter;
        auto DurationEnd = DurationCounter + Wave.Duration;
        if (DurationStart <= TimeSinceStart && TimeSinceStart < DurationEnd)
        {
            return Wave.PlayingState;
        }

        DurationCounter += Wave.Duration;
    }

    // The "Waves" configuration is malformed if we reach this point. Fix the config!
    check(false);
    return EChompGamePlayingState::None;
}

void AChompGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Meant for debugging current wave:
    DEBUG_LOG(TEXT("%f %d"), GetWorld()->GetTimeSeconds(), GetCurrentWave());

    // Compute the last known game playing state.
    auto CurrentWave = GetCurrentWave();

    // If there was a change in the last known game playing state, broadcast the event.
    if (LastKnownGamePlayingState != CurrentWave)
    {
        OnGamePlayingStateChangedDelegate.Broadcast(LastKnownGamePlayingState, CurrentWave);
    }

    // Afterward, save the new game playing state.
    LastKnownGamePlayingState = CurrentWave;
}
