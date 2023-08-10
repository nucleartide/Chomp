#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/ChompGameStateEnum.h"
#include "GameState/ChompPlayingSubstateEnum.h"
#include "Utils/IntFieldWithLastUpdatedTime.h"
#include "ChompGameState.generated.h"

struct FWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature,
                                            int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdatedSignature,
                                            int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature,
                                             EChompGameStateEnum, OldState,
                                             EChompGameStateEnum, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChangedSignature,
                                             EChompPlayingSubstateEnum, OldSubstate,
                                             EChompPlayingSubstateEnum, NewSubstate);

USTRUCT()
struct FCurrentSubstate
{
	GENERATED_BODY()

private:
	// The substate that the game was in last frame.
	UPROPERTY(VisibleInstanceOnly)
	EChompPlayingSubstateEnum LastPlayingSubstate = EChompPlayingSubstateEnum::None;

	// The time reported by UWorld::GetTimeSeconds when the game entered into an EChompGameStateEnum::Playing state.
	UPROPERTY(VisibleInstanceOnly)
	double PlayingStartTime = 0.0;

	// The time that the game entered into a Frightened substate,
	// during which Pacman has the opportunity to consume ghosts.
	UPROPERTY(VisibleInstanceOnly)
	double FrightenedStartTime = 0.0;

	// The accrued time spent in the frightened substate.
	// Needed to compute the current substate, which is a function of time and our FWaves configuration.
	UPROPERTY(VisibleInstanceOnly)
	double TimeSpentInFrightenedSubstate = 0.0;

	// Reference to the configured Waves of ghost behavior.
	TArray<FWave> Waves;

	// Reference to frightened substate duration.
	double FrightenedSubstateDuration;

public:
	[[nodiscard]] FCurrentSubstate(
		const EChompPlayingSubstateEnum LastPlayingSubstate,
		const double PlayingStartTime,
		const double FrightenedStartTime,
		const double TimeSpentInFrightenedSubstate,
		const TArray<FWave>& Waves,
		const double FrightenedSubstateDuration
	):
		LastPlayingSubstate(LastPlayingSubstate),
		PlayingStartTime(PlayingStartTime),
		FrightenedStartTime(FrightenedStartTime),
		TimeSpentInFrightenedSubstate(TimeSpentInFrightenedSubstate),
		Waves(Waves),
		FrightenedSubstateDuration(FrightenedSubstateDuration)
	{
	}

	double GetPlayingStartTime() const
	{
		return PlayingStartTime;
	}

	EChompPlayingSubstateEnum GetLastPlayingSubstate() const
	{
		return LastPlayingSubstate;
	}

	// TODO: Get the existing game features working first.
	void StartGame(double PlayingStartTime)
	{
		// Set playing start time.
		// ...
	}

	// TODO: Get the existing game features working first.
	void Tick(double WorldTimeInSeconds)
	{
		// Compute the current substate.
		// ...

		// Grab a reference to LastPlayingSubstate. We're about to overwrite it.
		// ...

		// Assign LastPlayingSubstate to the current substate.
		// ...

		// Return the current substate and reference to old substate to the caller.
		// ...
	}

	// TODO(2): Do this after the existing game features have been refactored to use this struct.
	// TODO: You will need to update the behavior of Tick() to take into account this Frightened interruption.
	void Frighten(double CurrentWorldTimeInSeconds)
	{
		// Enter the Frightened substate.
		// ...
	}

private:
	EChompPlayingSubstateEnum GetCurrentPlayingSubstate() const
	{
		auto TimeSinceStart = GetTimeSinceStart();
		/*
    	const auto World = GetWorld();
    	check(World);
    	return World->GetTimeSeconds() - CurrentSubstate.GetPlayingStartTime();
    	*/
		auto DurationCounter = 0.0;

		for (const auto& [PlayingState, Duration] : Waves)
		{
			if (Duration < 0.0f)
			{
				auto DurationStart = DurationCounter;
				return TimeSinceStart >= DurationStart ? PlayingState : EChompPlayingSubstateEnum::None;
			}

			auto DurationStart = DurationCounter;
			if (auto DurationEnd = DurationCounter + Duration; DurationStart <= TimeSinceStart && TimeSinceStart <
				DurationEnd)
			{
				return PlayingState;
			}

			DurationCounter += Duration;
		}

		checkf(false, TEXT("The 'Waves' configuration is malformed if we reach this point. Fix the config!"));
		return EChompPlayingSubstateEnum::None;
	}
};

UCLASS(Blueprintable)
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly)
	TArray<FWave> Waves;

	UPROPERTY(EditDefaultsOnly)
	double FrightenedSubstateDuration = 5.0;

	UPROPERTY(VisibleInstanceOnly)
	int Score = 0;

	UPROPERTY(VisibleInstanceOnly)
	int NumberOfDotsRemaining = 0;

	UPROPERTY(VisibleInstanceOnly)
	FIntFieldWithLastUpdatedTime NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime(0, nullptr);

	UPROPERTY(VisibleInstanceOnly)
	EChompGameStateEnum GameState = EChompGameStateEnum::None;

	UPROPERTY(VisibleInstanceOnly)
	FCurrentSubstate CurrentSubstate;

	void UpdateScore(int NewScore);

	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);

	float GetTimeSinceStart() const;

	void TransitionTo(EChompGameStateEnum NewState);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsClearedSignature OnDotsClearedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsConsumedUpdatedSignature OnDotsConsumedUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnScoreUpdatedSignature OnScoreUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGamePlayingStateChangedSignature OnGamePlayingStateChangedDelegate;

	AChompGameState();

	void ResetDots(int NumberOfDots);

	void ConsumeDot();

	void ConsumeEnergizerDot();

	EChompGameStateEnum GetEnum() const;

	int GetScore() const;

	FIntFieldWithLastUpdatedTime GetNumberOfDotsConsumed() const;

	void LoseGame();

	void StartGame();

	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);
};
