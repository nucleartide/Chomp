// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChompPlayingSubstateEnum.h"
#include "Wave.h"
#include "UObject/Object.h"
#include "CurrentSubstate.generated.h"

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
	double PlayingStartTime = std::numeric_limits<double>::max();

	// The time reported by UWorld::GetTimeSeconds when the game entered into an
	// EChompPlayingSubstateEnum::Frightened substate, during which Pacman has the opportunity to consume ghosts.
	UPROPERTY(VisibleInstanceOnly)
	double FrightenedStartTime = std::numeric_limits<double>::max();

	// The accrued time spent in the frightened substate.
	//
	// Needed to compute the current substate, which is a function of time, our FWaves configuration,
	// and the TimeSpentInFrightenedSubstate.
	UPROPERTY(VisibleInstanceOnly)
	double TimeSpentInFrightenedSubstate = 0.0;

	// Reference to the configured WavesRef of ghost behavior.
	//
	// Note: this value should be injected by constructor.
	TArray<FWave> WavesRef;

	// Reference to frightened substate duration.
	//
	// Note: this value should be injected by constructor.
	double FrightenedDurationRef = 0.0;

	EChompPlayingSubstateEnum GetCurrentPlayingSubstate(const double CurrentWorldTimeSeconds) const
	{
		// Early return if we are in a frightened substate.
		if (CurrentWorldTimeSeconds < FrightenedStartTime + FrightenedDurationRef)
			return EChompPlayingSubstateEnum::Frightened;

		const auto TimeSinceStart = CurrentWorldTimeSeconds - PlayingStartTime - TimeSpentInFrightenedSubstate;
		auto DurationStart = 0.0;

		for (const auto& [Substate, Duration] : WavesRef)
		{
			// Negative durations receive special treatment.
			//
			// To take an example, if "Chase" is the last element of WavesRef,
			// then the logic below will permanently return "Chase" once we've gone through the earlier waves.
			if (Duration < 0.0)
			{
				return TimeSinceStart >= DurationStart ? Substate : EChompPlayingSubstateEnum::None;
			}

			// Otherwise, if we are within this wave's [Start, End), return the substate.
			if (const auto DurationEnd = DurationStart + Duration;
				DurationStart <= TimeSinceStart && TimeSinceStart < DurationEnd)
			{
				return Substate;
			}

			// Bump our accumulation variable.
			DurationStart += Duration;
		}

		checkf(false, TEXT("The 'WavesRef' configuration is malformed if we reach this point. Fix the config!"));
		return EChompPlayingSubstateEnum::None;
	}

public:
	[[nodiscard]] FCurrentSubstate()
	{
	}

	[[nodiscard]] FCurrentSubstate(
		const TArray<FWave>& WavesRef,
		const double FrightenedSubstateDurationRef
	):
		FrightenedStartTime(-FrightenedSubstateDurationRef),
		WavesRef(WavesRef),
		FrightenedDurationRef(FrightenedSubstateDurationRef)
	{
	}

	void StartGame(const double CurrentWorldTimeSeconds)
	{
		check(GetCurrentPlayingSubstate(CurrentWorldTimeSeconds) != EChompPlayingSubstateEnum::Frightened);
		PlayingStartTime = CurrentWorldTimeSeconds;
	}

	// We keep a reference to the LastPlayingSubstate so that we can call a delegate when the substate changes.
	std::tuple<EChompPlayingSubstateEnum, EChompPlayingSubstateEnum> Tick(const double CurrentWorldTimeSeconds)
	{
		// Compute the current substate.
		const auto CurrentSubstate = GetCurrentPlayingSubstate(CurrentWorldTimeSeconds);

		// Grab a reference to LastPlayingSubstate. We're about to overwrite it.
		const auto OldLastPlayingSubstate = LastPlayingSubstate;

		// Assign LastPlayingSubstate to the current substate.
		LastPlayingSubstate = CurrentSubstate;

		// Return the current substate and reference to old substate to the caller.
		return std::make_tuple(OldLastPlayingSubstate, CurrentSubstate);
	}

	std::tuple<EChompPlayingSubstateEnum, EChompPlayingSubstateEnum> StopPlaying()
	{
		// We're switching to the "None" substate.
		constexpr auto CurrentSubstate = EChompPlayingSubstateEnum::None;

		// Grab a reference to LastPlayingSubstate. We're about to overwrite it.
		const auto OldLastPlayingSubstate = LastPlayingSubstate;

		// Assign LastPlayingSubstate to the current substate.
		LastPlayingSubstate = CurrentSubstate;

		// Return the current substate and reference to old substate to the caller.
		check(OldLastPlayingSubstate != CurrentSubstate);
		return std::make_tuple(OldLastPlayingSubstate, CurrentSubstate);
	}

	EChompPlayingSubstateEnum GetEnum(const double CurrentWorldTimeSeconds) const
	{
		return GetCurrentPlayingSubstate(CurrentWorldTimeSeconds);
	}

#if false
	// TODO(2): Do this after the existing game features have been refactored to use this struct.
	// TODO: You will need to update the behavior of Tick() to take into account this Frightened interruption.
	void Frighten(double CurrentWorldTimeInSeconds)
	{
		// Enter the Frightened substate.
		// ...
	}
#endif
};
