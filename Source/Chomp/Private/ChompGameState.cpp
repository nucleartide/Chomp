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

void AChompGameState::ConsumeEnergizerDot()
{
	UpdateScore(Score + 5 * ScoreMultiplier);
	const auto World = FSafeGet::World(this);
	CurrentSubstate.Frighten(World->GetTimeSeconds());
}

void AChompGameState::ConsumeGhost()
{
	// Pre-conditions.
	const auto World = FSafeGet::World(this);
	const auto OldNumGhostsConsumed = CurrentSubstate.GetNumGhostsConsumed(World->GetTimeSeconds());
	check(OldNumGhostsConsumed >= 0);

	// Bump.
	CurrentSubstate.IncrementNumGhostsConsumed();

	// Get new value.
	const auto NumGhostsConsumed = CurrentSubstate.GetNumGhostsConsumed(World->GetTimeSeconds());
	check(NumGhostsConsumed == OldNumGhostsConsumed + 1);

	// NumGhostsConsumed == 1, 2, 3, 4
	// GhostScalingFactor == 2**1, 2**2, 2**3, 2**4
	// DesiredScore == 200, 400, 800, 1600
	const auto GhostScalingFactor = FMath::Pow(2.0, NumGhostsConsumed) * 10;
	UpdateScore(Score + GhostScalingFactor * ScoreMultiplier);
}

void AChompGameState::UpdateScore(const int NewScore)
{
	Score = NewScore;
	OnScoreUpdated.Broadcast(NewScore);
}

void AChompGameState::UpdateNumberOfDotsRemaining(const int NewNumberOfDotsRemaining)
{
	NumberOfDotsRemaining = NewNumberOfDotsRemaining;
	if (NumberOfDotsRemaining == 0)
	{
		OnDotsCleared.Broadcast();
		TransitionTo(EChompGameStateEnum::GameOverWin);
	}
}

void AChompGameState::UpdateNumberOfLives(const int NewNumOfLives)
{
	const auto Bounded = FMath::Max(NewNumOfLives, 0);
	NumberOfLives = Bounded;
	OnLivesChanged.Broadcast(Bounded);
}

void AChompGameState::UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed)
{
	const auto World = FSafeGet::World(this);
	NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime(NewNumberOfDotsConsumed, World);
	OnDotsConsumedUpdated.Broadcast(NewNumberOfDotsConsumed);
}

EChompPlayingSubstateEnum AChompGameState::GetSubstateEnum(const bool ExcludeFrightened) const
{
	const auto World = FSafeGet::World(this);
	return CurrentSubstate.GetEnum(World->GetTimeSeconds(), ExcludeFrightened);
}

void AChompGameState::LoseLife()
{
	UpdateNumberOfLives(NumberOfLives - 1);
	
	// if number of lives is zero,
	//     then LoseGame()
	//     make LoseGame() private
	// else,
	//     kick off a timer for 3s
	//     once timer is elapsed, ResetRound() (keep the dots, reset the ghost state, reset the player)
}

void AChompGameState::LoseGame()
{
	TransitionTo(EChompGameStateEnum::GameOverLose);
}

void AChompGameState::StartGame()
{
	// Pre-conditions.
	check(Waves.Num() > 0);
	
	CurrentSubstate = FCurrentSubstate(Waves, FrightenedSubstateDuration);
	CurrentSubstate.StartGame(GetWorld()->GetTimeSeconds());

	UpdateNumberOfLives(StartingNumberOfLives);
	
	TransitionTo(EChompGameStateEnum::Playing);
}

void AChompGameState::TransitionTo(EChompGameStateEnum NewState)
{
	// Pre-conditions.
	const auto OldState = GameState;
	check(OldState != NewState);

	GameState = NewState;
	OnGameStateChanged.Broadcast(OldState, NewState);

	if (NewState != EChompGameStateEnum::Playing)
	{
		const auto World = FSafeGet::World(this);
		const auto [OldSubstate, NewSubstate] = CurrentSubstate.StopPlaying(World->GetTimeSeconds());
		check(OldSubstate != NewSubstate);
		OnGamePlayingStateChanged.Broadcast(OldSubstate, NewSubstate);
	}
}

EChompGameStateEnum AChompGameState::GetEnum() const
{
	return GameState;
}

bool AChompGameState::IsPlaying() const
{
	return GameState == EChompGameStateEnum::Playing;
}

bool AChompGameState::IsFrightened() const
{
	return IsPlaying() && GetSubstateEnum() == EChompPlayingSubstateEnum::Frightened;
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
			OnGamePlayingStateChanged.Broadcast(OldSubstate, NewSubstate);
	}
}

FIntFieldWithLastUpdatedTime AChompGameState::GetNumberOfDotsConsumed() const
{
	return NumberOfDotsConsumed;
}
