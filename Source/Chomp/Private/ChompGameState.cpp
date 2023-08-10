#include "ChompGameState.h"
#include "Utils/Debug.h"
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

void AChompGameState::ConsumeEnergizerDot()
{
	DEBUG_LOG(TEXT("Consumed energizer dot."));
	UpdateScore(Score + ScoreMultiplier * 10);

	// TODO.
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

EChompPlayingSubstateEnum AChompGameState::GetSubstateEnum() const
{
	const auto World = FSafeGet::World(this);
	return CurrentSubstate.GetEnum(World->GetTimeSeconds());
}

void AChompGameState::LoseGame()
{
	TransitionTo(EChompGameStateEnum::GameOverLose);
}

void AChompGameState::StartGame()
{
	// Initialize CurrentSubstate.
	check(Waves.Num() > 0);
	CurrentSubstate = FCurrentSubstate(Waves, FrightenedSubstateDuration);
	CurrentSubstate.StartGame(GetWorld()->GetTimeSeconds());

	// Transition to Playing substate.
	TransitionTo(EChompGameStateEnum::Playing);
}

void AChompGameState::TransitionTo(EChompGameStateEnum NewState)
{
	auto OldState = GameState;
	check(OldState != NewState);

	GameState = NewState;
	OnGameStateChangedDelegate.Broadcast(OldState, NewState);

	if (NewState != EChompGameStateEnum::Playing)
	{
		const auto World = FSafeGet::World(this);
		const auto [OldSubstate, NewSubstate] = CurrentSubstate.StopPlaying(World->GetTimeSeconds());
		check(OldSubstate != NewSubstate);
		OnGamePlayingStateChangedDelegate.Broadcast(OldSubstate, NewSubstate);
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
		const auto World = FSafeGet::World(this);
		const auto [OldSubstate, NewSubstate] = CurrentSubstate.Tick(World->GetTimeSeconds());
		if (OldSubstate != NewSubstate)
			OnGamePlayingStateChangedDelegate.Broadcast(OldSubstate, NewSubstate);
	}
}

FIntFieldWithLastUpdatedTime AChompGameState::GetNumberOfDotsConsumed() const
{
	return NumberOfDotsConsumed;
}
