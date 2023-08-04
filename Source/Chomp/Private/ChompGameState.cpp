#include "ChompGameState.h"

#include "Utils/SafeGet.h"

AChompGameState::AChompGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChompGameState::ResetDots(const int NumberOfDots)
{
	UpdateScore(0);
	UpdateNumberOfDotsRemaining(NumberOfDots);
	UpdateNumberOfDotsConsumed(0);
}

void AChompGameState::ConsumeDot()
{
	UpdateScore(Score + ScoreMultiplier);
	UpdateNumberOfDotsRemaining(NumberOfDotsRemaining - 1);
	UpdateNumberOfDotsConsumed(NumberOfDotsConsumed.GetValue() + 1);
}

void AChompGameState::UpdateScore(const int NewScore)
{
	Score = NewScore;
	OnScoreUpdatedDelegate.Broadcast(NewScore);
}

void AChompGameState::UpdateNumberOfDotsRemaining(const int NewNumberOfDotsRemaining)
{
	NumberOfDotsRemaining = NewNumberOfDotsRemaining;
	if (NumberOfDotsRemaining == 0)
	{
		OnDotsClearedDelegate.Broadcast();
		TransitionTo(EChompGameState::GameOverWin);
	}
}

void AChompGameState::UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed)
{
	const auto World = FSafeGet::World(this);
	NumberOfDotsConsumed = TFieldWithLastUpdatedTime(NewNumberOfDotsConsumed, World);
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

	if (NewState != EChompGameState::Playing)
	{
		auto OldGamePlayingState = LastKnownGamePlayingSubstate;
		auto NewGamePlayingState = EChompGamePlayingSubstate::None;
		check(OldGamePlayingState != NewGamePlayingState);
		LastKnownGamePlayingSubstate = NewGamePlayingState;
		OnGamePlayingStateChangedDelegate.Broadcast(OldGamePlayingState, NewGamePlayingState);
	}
}

EChompGameState AChompGameState::GetEnum() const
{
	return GameState;
}

int AChompGameState::GetScore() const
{
	return Score;
}

EChompGamePlayingSubstate AChompGameState::GetPlayingSubstate() const
{
	auto TimeSinceStart = GetTimeSinceStart();
	auto DurationCounter = 0.0;

	for (const auto& [PlayingState, Duration] : Waves)
	{
		if (Duration < 0.0f)
		{
			auto DurationStart = DurationCounter;
			return TimeSinceStart >= DurationStart ? PlayingState : EChompGamePlayingSubstate::None;
		}

		auto DurationStart = DurationCounter;
		if (auto DurationEnd = DurationCounter + Duration; DurationStart <= TimeSinceStart && TimeSinceStart <
			DurationEnd)
		{
			return PlayingState;
		}

		DurationCounter += Duration;
	}

	// The "Waves" configuration is malformed if we reach this point. Fix the config!
	check(false);
	return EChompGamePlayingSubstate::None;
}

void AChompGameState::BeginPlay()
{
	Super::BeginPlay();
	StartGame();
}

void AChompGameState::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GameState == EChompGameState::Playing)
	{
		// Compute the last known game playing state.
		const auto CurrentWave = GetPlayingSubstate();

		// If there was a change in the last known game playing state, broadcast the event.
		if (LastKnownGamePlayingSubstate != CurrentWave)
			OnGamePlayingStateChangedDelegate.Broadcast(LastKnownGamePlayingSubstate, CurrentWave);

		// Afterward, save the new game playing state.
		LastKnownGamePlayingSubstate = CurrentWave;
	}
}

float AChompGameState::GetTimeSinceStart() const
{
	const auto World = GetWorld();
	check(World);
	return World->GetTimeSeconds() - GameStartTime;
}

TFieldWithLastUpdatedTime<int> AChompGameState::GetNumberOfDotsConsumed() const
{
	return NumberOfDotsConsumed;
}
