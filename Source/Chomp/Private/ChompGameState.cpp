#include "ChompGameState.h"
#include "UE5Coro.h"
#include "GameState/ChompSaveGame.h"
#include "GameState/LocalStorageSubsystem.h"
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
	CurrentSubstate.Frighten(World);
}

void AChompGameState::ConsumeGhost()
{
	// Pre-conditions.
	const auto World = FSafeGet::World(this);
	const auto OldNumGhostsConsumed = CurrentSubstate.GetNumGhostsConsumed();
	check(OldNumGhostsConsumed >= 0);

	// Bump.
	CurrentSubstate.ConsumeGhost();

	// Get new value.
	const auto NumGhostsConsumed = CurrentSubstate.GetNumGhostsConsumed();
	check(NumGhostsConsumed == OldNumGhostsConsumed + 1);

	// NumGhostsConsumed == 1, 2, 3, 4
	// GhostScalingFactor == 2**1, 2**2, 2**3, 2**4
	// DesiredScore == 200, 400, 800, 1600
	const auto GhostScalingFactor = FMath::Pow(2.0, NumGhostsConsumed) * 10;
	UpdateScore(Score + GhostScalingFactor * ScoreMultiplier);
}

void AChompGameState::UpdateScore(const int NewScore)
{
	// Update OneUpCounter.
	const auto Diff = NewScore - Score;
	OneUpCounter += Diff;

	if (OneUpCounter >= OneUpThreshold && NumberOfLives < StartingNumberOfLives)
	{
		// Award a life.
		UpdateNumberOfLives(NumberOfLives + 1);

		// Zero out counter.
		OneUpCounter = 0;

		// Emit a message.
		OnOneUp.Broadcast();
	}

	// Update Score.
	Score = NewScore;
	const auto SaveGame = GetGameInstance()->GetSubsystem<ULocalStorageSubsystem>()->GetSaveGame();
	SaveGame->SetHighScore(Score, CurrentLevel);
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

EChompPlayingSubstateEnum AChompGameState::GetSubstateEnum(const bool GetUnderlyingSubstate) const
{
	return CurrentSubstate.GetEnum(GetUnderlyingSubstate);
}

UE5Coro::TCoroutine<> AChompGameState::LoseLife()
{
	UpdateNumberOfLives(NumberOfLives - 1);
	if (NumberOfLives == 0)
	{
		TransitionTo(EChompGameStateEnum::GameOverLose);
	}
	else
	{
		TransitionTo(EChompGameStateEnum::LostLife);
		co_await UE5Coro::Latent::Seconds(3.0);
		TransitionTo(EChompGameStateEnum::Playing);
	}
}

void AChompGameState::StartGame()
{
	// Pre-conditions.
	check(Waves.Num() > 0);

	CurrentSubstate = FChompPlayingSubstate(FrightenedSubstateDuration, Waves);
	const auto World = FSafeGet::World(this);
	CurrentSubstate.Start(World);

	UpdateNumberOfLives(StartingNumberOfLives);

	TransitionTo(EChompGameStateEnum::Playing);
}

void AChompGameState::TransitionTo(EChompGameStateEnum NewState)
{
	// Pre-conditions.
	const auto OldState = GameState;
	check(OldState != NewState);
	const auto OldOneUpCounter = OneUpCounter;

	GameState = NewState;
	OnGameStateChanged.Broadcast(OldState, NewState);

	if (NewState != EChompGameStateEnum::Playing)
	{
		CurrentSubstate.Stop();
		OnGamePlayingStateChanged.Broadcast(LastSubstateEnum, EChompPlayingSubstateEnum::None);
		LastSubstateEnum = EChompPlayingSubstateEnum::None;
	}
	else if (NewState == EChompGameStateEnum::Playing)
	{
		OneUpCounter = 0;
	}

	// Post-conditions.
	check(
		NewState != EChompGameStateEnum::Playing
		? !CurrentSubstate.IsRunning()
		: true
	);
	check(
		NewState == EChompGameStateEnum::Playing
		? OneUpCounter == 0
		: OldOneUpCounter == OneUpCounter
	);
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
	check(CurrentLevel);
}

void AChompGameState::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GameState == EChompGameStateEnum::Playing)
	{
		if (const auto NewSubstate = CurrentSubstate.GetEnum(); LastSubstateEnum != NewSubstate)
		{
			OnGamePlayingStateChanged.Broadcast(LastSubstateEnum, NewSubstate);
			LastSubstateEnum = NewSubstate;
		}
	}
}

FIntFieldWithLastUpdatedTime AChompGameState::GetNumberOfDotsConsumed() const
{
	return NumberOfDotsConsumed;
}
