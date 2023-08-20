#pragma once

#include "CoreMinimal.h"
#include "ChompPlayingSubstateEnum.h"
#include "UE5Coro.h"
#include "Wave.h"
#include "UObject/Object.h"
#include "ChompPlayingSubstate.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChanged,
                                             EChompPlayingSubstateEnum, OldSubstate,
                                             EChompPlayingSubstateEnum, NewSubstate);

USTRUCT()
struct FChompPlayingSubstate
{
	GENERATED_BODY()

	/**
	 * Mutable state.
	 */

private:
	// Reference to duration of time to spend in frightened substate.
	double FrightenedDurationRef = 0.0;

	// The configured Waves of ghost behavior.
	TArray<FWave> Waves;

	// Index of the current wave.
	int CurrentWaveIndex = 0;

	// Start time of current wave.
	double CurrentWaveStartTime = 0.0;

	// Coroutine for when game is in frightened substate.
	UE5Coro::TCoroutine<> Frightened = UE5Coro::TCoroutine<>::FromResult(nullptr);

	// Coroutine to kick off timer for current wave.
	UE5Coro::TCoroutine<> CurrentWave = UE5Coro::TCoroutine<>::FromResult(nullptr);

	// If Frightened is active, this field will contain
	// the number of ghosts consumed within the current frightened duration.
	//
	// Will reset upon completion of Frightened.
	int NumGhostsConsumed = 0;

public:
	FOnChanged OnChanged;

	/**
	 * Private methods.
	 */

private:
	UE5Coro::TCoroutine<> FrightenAsync(const UWorld* WorldInstance)
	{
		// Pre-conditions.
		const auto OldDuration = Waves[CurrentWaveIndex].Duration;

		// Stop CurrentWave temporarily.
		if (!CurrentWave.IsDone())
		{
			CurrentWave.Cancel();
			if (Waves[CurrentWaveIndex].Duration > 0.0)
				Waves[CurrentWaveIndex].Duration = WorldInstance->GetTimeSeconds() - CurrentWaveStartTime;
		}

		// Wait.
		OnChanged.Broadcast(GetEnum(true), EChompPlayingSubstateEnum::Frightened);
		co_await UE5Coro::Latent::Seconds(FrightenedDurationRef);
		OnChanged.Broadcast(EChompPlayingSubstateEnum::Frightened, GetEnum(true));

		// When frightened substate is over, reset the number of ghosts.
		NumGhostsConsumed = 0;

		// Also reboot the CurrentWave coroutine.
		CurrentWave = StartAsync(WorldInstance, CurrentWaveIndex);

		// Post-conditions.
		check(NumGhostsConsumed == 0);
		check(!CurrentWave.IsDone());
		check(OldDuration < 0.0 ? OldDuration == Waves[CurrentWaveIndex].Duration : true);
	}

	UE5Coro::TCoroutine<> StartAsync(const UWorld* WorldInstance, const int StartIndex = 0)
	{
		for (CurrentWaveIndex = StartIndex; CurrentWaveIndex < Waves.Num(); CurrentWaveIndex++)
		{
			CurrentWaveStartTime = WorldInstance->GetTimeSeconds();
			if (const auto& [_, Duration] = Waves[CurrentWaveIndex]; Duration >= 0.0)
				co_await UE5Coro::Latent::Seconds(Duration);
			Waves[CurrentWaveIndex].Duration = 0.0;
		}

		// Post-conditions.
		check(CurrentWaveIndex == Waves.Num());
		check(CurrentWaveStartTime < WorldInstance->GetTimeSeconds());
		for (const auto& [_, Duration] : Waves)
			check(FMath::IsNearlyZero(Duration));
	}

	/**
	 * Public API.
	 */

public:
	[[nodiscard]] FChompPlayingSubstate()
	{
	}

	[[nodiscard]] FChompPlayingSubstate(
		const double FrightenedDurationRef,
		const TArray<FWave>& WavesRef
	):
		FrightenedDurationRef(FrightenedDurationRef),
		Waves(WavesRef)
	{
		// Post-conditions.
		check(this->Waves.Num() > 0);
		check(this->FrightenedDurationRef > 0.0);
	}

	void Frighten(const UWorld* WorldInstance)
	{
		// Stop old coroutine.
		if (!Frightened.IsDone())
			Frightened.Cancel();

		// Start new coroutine.
		Frightened = FrightenAsync(WorldInstance);

		// Post-conditions.
		check(!Frightened.IsDone());
	}

	EChompPlayingSubstateEnum GetEnum(const bool GetUnderlyingSubstate = false) const
	{
		// Pre-conditions.
		check(0 <= CurrentWaveIndex && CurrentWaveIndex < Waves.Num());

		if (!GetUnderlyingSubstate && !Frightened.IsDone())
			return EChompPlayingSubstateEnum::Frightened;

		const auto [PlayingState, Duration] = Waves[CurrentWaveIndex];
		return PlayingState;
	}

	void Start(UWorld* WorldInstance)
	{
		// Pre-conditions.
		check(GetEnum() != EChompPlayingSubstateEnum::Frightened);
		check(CurrentWave.IsDone());

		CurrentWave = FrightenAsync(WorldInstance);

		// Post-conditions.
		check(GetEnum() != EChompPlayingSubstateEnum::Frightened);
	}

	void Stop()
	{
		NumGhostsConsumed = 0;

		if (!Frightened.IsDone())
			Frightened.Cancel();

		if (!CurrentWave.IsDone())
			CurrentWave.Cancel();
	}

	void ConsumeGhost()
	{
		NumGhostsConsumed++;
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

	// TODO: Include alerts to changes
};
