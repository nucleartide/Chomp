#include "ChompGameState.h"
#include "Utils/Debug.h"

AChompGameState::AChompGameState()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompGameState::ResetDots(int NumberOfDots)
{
    UpdateScore(0);
    UpdateNumberOfDotsRemaining(NumberOfDots);
    UpdateNumberOfDotsConsumed(0);
}

void AChompGameState::ConsumeDot()
{
    UpdateScore(Score + ScoreMultiplier);
    UpdateNumberOfDotsRemaining(NumberOfDotsRemaining - 1);
    UpdateNumberOfDotsConsumed(NumberOfDotsConsumed + 1);
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

void AChompGameState::UpdateNumberOfDotsConsumed(int NewNumberOfDotsConsumed)
{
    NumberOfDotsConsumed = NewNumberOfDotsConsumed;
    OnDotsConsumedUpdatedDelegate.Broadcast(NewNumberOfDotsConsumed);
}

void AChompGameState::LoseGame()
{
    TransitionTo(EChompGameState::GameOverLose);
}

void AChompGameState::StartGame()
{
    TransitionTo(EChompGameState::Playing);
    GameStartTime = GetWorld()->GetTimeSeconds();
}

void AChompGameState::TransitionTo(EChompGameState NewState)
{
    auto OldState = GameState;
    check(OldState != NewState);

    GameState = NewState;
    OnGameStateChangedDelegate.Broadcast(OldState, NewState);
    OnLateGameStateChangedDelegate.Broadcast(OldState, NewState);

    if (GameState != EChompGameState::Playing)
    {
        auto OldGamePlayingState = LastKnownGamePlayingState;
        auto NewGamePlayingState = EChompGamePlayingState::None;
        check(OldGamePlayingState != NewGamePlayingState);

        LastKnownGamePlayingState = NewGamePlayingState;
        OnGamePlayingStateChangedDelegate.Broadcast(OldGamePlayingState, NewGamePlayingState);
    }
}

EChompGameState AChompGameState::GetEnum()
{
    return GameState;
}

int AChompGameState::GetScore()
{
    return Score;
}

EChompGamePlayingState AChompGameState::GetPlayingSubstate()
{
    auto TimeSinceStart = GetTimeSinceStart();
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

void AChompGameState::BeginPlay()
{
    Super::BeginPlay();
    StartGame();
}

void AChompGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GameState == EChompGameState::Playing)
    {
        // Compute the last known game playing state.
        auto CurrentWave = GetPlayingSubstate();

        // If there was a change in the last known game playing state, broadcast the event.
        if (LastKnownGamePlayingState != CurrentWave)
            OnGamePlayingStateChangedDelegate.Broadcast(LastKnownGamePlayingState, CurrentWave);

        // Afterward, save the new game playing state.
        LastKnownGamePlayingState = CurrentWave;
    }
}

float AChompGameState::GetTimeSinceStart()
{
    auto World = GetWorld();
    return World->GetTimeSeconds() - GameStartTime;
}

int AChompGameState::GetNumberOfDotsConsumed()
{
    return NumberOfDotsConsumed;
}
