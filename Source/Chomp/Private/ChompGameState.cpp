#include "ChompGameState.h"

#include "GameState/Wave.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

AChompGameState::AChompGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	checkf(false, TEXT("hmm, does CurrentSubstate have a default constructor? why isn't Rider complaining?"))
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

void AChompGameState::ConsumeEnergizerDot()
{
	DEBUG_LOG(TEXT("Consumed energizer dot."));
	UpdateScore(Score + ScoreMultiplier * 10);

#if false
	UPROPERTY(VisibleInstanceOnly)
	EChompPlayingSubstateEnum LastKnownGamePlayingSubstate = EChompPlayingSubstateEnum::None;
#endif
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
		TransitionTo(EChompGameStateEnum::GameOverWin);
	}
}

void AChompGameState::UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed)
{
	const auto World = FSafeGet::World(this);
	NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime(NewNumberOfDotsConsumed, World);
	OnDotsConsumedUpdatedDelegate.Broadcast(NewNumberOfDotsConsumed);
}

void AChompGameState::LoseGame()
{
	TransitionTo(EChompGameStateEnum::GameOverLose);
}

void AChompGameState::StartGame()
{
	TransitionTo(EChompGameStateEnum::Playing);
	CurrentSubstate.SetPlayingStartTime(GetWorld()->GetTimeSeconds());
}

void AChompGameState::TransitionTo(EChompGameStateEnum NewState)
{
	auto OldState = GameState;
	check(OldState != NewState);

	GameState = NewState;
	OnGameStateChangedDelegate.Broadcast(OldState, NewState);

	if (NewState != EChompGameStateEnum::Playing)
	{
		auto OldGamePlayingState = CurrentSubstate.GetLastPlayingSubstate();
		auto NewGamePlayingState = EChompPlayingSubstateEnum::None;
		check(OldGamePlayingState != NewGamePlayingState);
		CurrentSubstate.SetLastPlayingSubstate(NewGamePlayingState);
		OnGamePlayingStateChangedDelegate.Broadcast(OldGamePlayingState, NewGamePlayingState);
	}
}

EChompGameStateEnum AChompGameState::GetEnum() const
{
	return GameState;
}

int AChompGameState::GetScore() const
{
	return Score;
}

void AChompGameState::BeginPlay()
{
	Super::BeginPlay();
	StartGame();
}

void AChompGameState::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GameState == EChompGameStateEnum::Playing)
	{
		// Compute the last known game playing state.
		const auto CurrentWave = GetPlayingSubstate();

		// If there was a change in the last known game playing state, broadcast the event.
		if (CurrentSubstate.GetLastPlayingSubstate() != CurrentWave)
			OnGamePlayingStateChangedDelegate.Broadcast(CurrentSubstate.GetLastPlayingSubstate(), CurrentWave);

		// Afterward, save the new game playing state.
		CurrentSubstate.SetLastPlayingSubstate(CurrentWave);
	}
}

FIntFieldWithLastUpdatedTime AChompGameState::GetNumberOfDotsConsumed() const
{
	return NumberOfDotsConsumed;
}
