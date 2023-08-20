#pragma once

#include "CoreMinimal.h"
#include "ChompPlayingSubstateEnum.h"
#include "UE5Coro.h"
#include "Wave.h"
#include "Misc/Optional.h"
#include "UObject/Object.h"
#include "ChompPlayingSubstate.generated.h"

USTRUCT()
struct FChompPlayingSubstate
{
	GENERATED_BODY()

private:
	// The duration of time to spend in the EChompPlayingSubstateEnum::Frightened substate.
	UPROPERTY(VisibleInstanceOnly)
	double FrightenedDurationRef = 0.0;

	// The configured WavesRef of ghost behavior.
	UPROPERTY(VisibleInstanceOnly)
	TArray<FWave> WavesRef;

	// Corresponding countdown timers for the configured WavesRef.
	UPROPERTY(VisibleInstanceOnly)
	TArray<double> WaveCountdownTimers;

	// Index of the current wave.
	UPROPERTY(VisibleInstanceOnly)
	int CurrentWaveIndex = 0;

	// Start time of current wave, as reported by UWorld::GetTimeSeconds.
	UPROPERTY(VisibleInstanceOnly)
	double CurrentWaveStartTime = 0.0;

	// Coroutine for when game is in the EChompPlayingSubstateEnum::Frightened substate.
	UE5Coro::TCoroutine<> Frightened = UE5Coro::TCoroutine<>::FromResult(nullptr);

	// Coroutine for progressing through the configured WavesRef.
	UE5Coro::TCoroutine<> WaveProgression = UE5Coro::TCoroutine<>::FromResult(nullptr);

	// If the Frightened coroutine is running, this field will contain the number of ghosts consumed within the
	// current frightened duration.
	//
	// Resets upon completion of the Frightened coroutine.
	UPROPERTY(VisibleInstanceOnly)
	int NumGhostsConsumed = 0;

	UE5Coro::TCoroutine<> FrightenAsync(const UWorld* WorldInstance)
	{
		// Pre-conditions.
		check(0 <= CurrentWaveIndex && CurrentWaveIndex < WavesRef.Num());

		// Pause the current wave.
		if (!WaveProgression.IsDone())
		{
			WaveProgression.Cancel();
			if (const auto Timer = WaveCountdownTimers[CurrentWaveIndex]; Timer > 0.0)
			{
				const auto ElapsedTime = WorldInstance->GetTimeSeconds() - CurrentWaveStartTime;
				WaveCountdownTimers[CurrentWaveIndex] = Timer - ElapsedTime;
			}
		}

		// Wait.
		co_await UE5Coro::Latent::Seconds(FrightenedDurationRef);

		// When frightened substate is over, reset the number of ghosts.
		NumGhostsConsumed = 0;

		// Also reboot the WaveProgression coroutine.
		WaveProgression = StartAsync(WorldInstance, CurrentWaveIndex);

		// Post-conditions.
		const auto DebugThis = this;
		for (auto i = 0; i < WaveCountdownTimers.Num(); i++)
		{
			if (const auto [_, Duration] = WavesRef[CurrentWaveIndex]; Duration > 0.0)
			{
				check(WaveCountdownTimers[CurrentWaveIndex] <= Duration);
			}
		}
		check(NumGhostsConsumed == 0);
	}

	UE5Coro::TCoroutine<> StartAsync(const UWorld* WorldInstance, const int StartIndex)
	{
		// Pre-conditions.
		check(StartIndex >= 0);

		// Reset timers.
		if (StartIndex == 0)
		{
			for (auto i = 0; i < WavesRef.Num(); i++)
			{
				const auto [_, Duration] = WavesRef[i];
				WaveCountdownTimers[i] = Duration;
			}
		}

		// Progress through waves.
		//
		// Note that it doesn't matter what the last wave is.
		for (CurrentWaveIndex = StartIndex; CurrentWaveIndex < WavesRef.Num() - 1; CurrentWaveIndex++)
		{
			CurrentWaveStartTime = WorldInstance->GetTimeSeconds();

			if (const auto Timer = WaveCountdownTimers[CurrentWaveIndex]; Timer > 0.0)
			{
				co_await UE5Coro::Latent::Seconds(Timer);
				WaveCountdownTimers[CurrentWaveIndex] = 0.0;
			}
		}

		// Post-conditions.
		const auto DebugThis = this;
		check(FrightenedDurationRef > 0.0);
		checkf(CurrentWaveIndex == WavesRef.Num() - 1, TEXT("Should be last index."));
		check(CurrentWaveStartTime <= WorldInstance->GetTimeSeconds());
		for (auto i = 0; i < WaveCountdownTimers.Num() - 1; i++)
		{
			const auto Timer = WaveCountdownTimers[i];
			check(Timer <= 0.0);
		}
	}

public:
	[[nodiscard]] FChompPlayingSubstate()
	{
	}

	[[nodiscard]] FChompPlayingSubstate(
		const double FrightenedDurationRef,
		const TArray<FWave>& WavesRef
	):
		FrightenedDurationRef(FrightenedDurationRef),
		WavesRef(WavesRef)
	{
		// Reset timers.
		for (const auto& [_, Duration] : WavesRef)
			WaveCountdownTimers.Add(Duration);

		// Post-conditions.
		check(this->FrightenedDurationRef > 0.0);
#ifndef WITH_EDITOR
		check(this->WavesRef.Num() > 0);
		check(this->WaveCountdownTimers.Num() == this->WavesRef.Num());
#endif
	}

	void Frighten(const UWorld* WorldInstance)
	{
		if (!Frightened.IsDone())
			Frightened.Cancel();

		Frightened = FrightenAsync(WorldInstance);

		// Post-conditions.
		check(!Frightened.IsDone());
	}

	EChompPlayingSubstateEnum GetEnum(const bool GetUnderlyingSubstate = false) const
	{
		// Pre-conditions.
		check(0 <= CurrentWaveIndex && CurrentWaveIndex < WavesRef.Num());

		if (!GetUnderlyingSubstate && !Frightened.IsDone())
			return EChompPlayingSubstateEnum::Frightened;

		const auto [PlayingState, Duration] = WavesRef[CurrentWaveIndex];
		return PlayingState;
	}

	void Start(UWorld* WorldInstance)
	{
		// Pre-conditions.
		check(GetEnum() != EChompPlayingSubstateEnum::Frightened);
		checkf(WaveProgression.IsDone(), TEXT("Should not have already started the wave progression."));

		WaveProgression = StartAsync(WorldInstance, 0);

		// Post-conditions.
		check(GetEnum() != EChompPlayingSubstateEnum::Frightened);
		check(CurrentWaveIndex == 0);
	}

	void Stop()
	{
		NumGhostsConsumed = 0;

		if (!Frightened.IsDone())
			Frightened.Cancel();

		if (!WaveProgression.IsDone())
			WaveProgression.Cancel();

		// Post-conditions.
		check(NumGhostsConsumed == 0);
	}

	void ConsumeGhost()
	{
		// Pre-conditions.
		const auto OldNumGhostsConsumed = NumGhostsConsumed;

		NumGhostsConsumed++;

		// Post-conditions.
		check(NumGhostsConsumed == OldNumGhostsConsumed + 1);
	}

	int GetNumGhostsConsumed() const
	{
		// Pre-conditions.
		check(
			GetEnum() != EChompPlayingSubstateEnum::Frightened
			? NumGhostsConsumed == 0
			: NumGhostsConsumed >= 0
		);

		return NumGhostsConsumed;
	}

	FChompPlayingSubstate& operator=(const FChompPlayingSubstate& Other)
	{
		if (this == &Other)
			return *this;
		FrightenedDurationRef = Other.FrightenedDurationRef;
		WavesRef = Other.WavesRef;
		WaveCountdownTimers = Other.WaveCountdownTimers;
		CurrentWaveIndex = Other.CurrentWaveIndex;
		CurrentWaveStartTime = Other.CurrentWaveStartTime;
		Frightened = Other.Frightened;
		WaveProgression = Other.WaveProgression;
		NumGhostsConsumed = Other.NumGhostsConsumed;
		return *this;
	}
};
