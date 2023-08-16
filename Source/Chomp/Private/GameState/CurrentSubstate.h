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
	// The substate that the game was in last tick.
	UPROPERTY(VisibleInstanceOnly)
	EChompPlayingSubstateEnum LastPlayingSubstate = EChompPlayingSubstateEnum::None;

	// The time reported by UWorld::GetTimeSeconds when the game entered into an EChompGameStateEnum::Playing state.
	UPROPERTY(VisibleInstanceOnly)
	double PlayingStartTime = 0.0;

	// The time reported by UWorld::GetTimeSeconds when the game entered into an
	// EChompPlayingSubstateEnum::Frightened substate, during which Pacman has the opportunity to consume ghosts.
	UPROPERTY(VisibleInstanceOnly)
	double FrightenedStartTime = 0.0;

	// The accrued time spent in the frightened substate.
	//
	// Needed to compute the current substate, which is a function of time, our FWaves configuration,
	// and the TimeSpentInFrightenedSubstate.
	UPROPERTY(VisibleInstanceOnly)
	double TimeSpentInFrightenedSubstate = 0.0;

	// The number of ghosts consumed within the current frightened substate.
	//
	// Will reset upon transitioning out of the frightened substate.
	int NumGhostsConsumed = 0;

	// Reference to the configured WavesRef of ghost behavior.
	//
	// Note: this value should be injected by constructor, and should not be mutated once set.
	TArray<FWave> WavesRef;

	// Reference to duration of time to spend in frightened substate.
	//
	// Note: this value should be injected by constructor, and should not be mutated once set.
	double FrightenedDurationRef = 0.0;

public:
	[[nodiscard]] FCurrentSubstate()
	{
	}

	[[nodiscard]] FCurrentSubstate(
		const TArray<FWave>& WavesRef,
		const double FrightenedDurationRef
	):
		WavesRef(WavesRef),
		FrightenedDurationRef(FrightenedDurationRef)
	{
	}

	EChompPlayingSubstateEnum GetEnum(const double CurrentWorldTimeSeconds, const bool ExcludeFrightened = false) const
	{
		// Early return if we are in a frightened substate.
		if (!ExcludeFrightened && CurrentWorldTimeSeconds < FrightenedStartTime + FrightenedDurationRef)
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

	void StartGame(const double CurrentWorldTimeSeconds)
	{
		// Pre-conditions.
		check(GetEnum(CurrentWorldTimeSeconds) != EChompPlayingSubstateEnum::Frightened);

		LastPlayingSubstate = EChompPlayingSubstateEnum::None;
		PlayingStartTime = CurrentWorldTimeSeconds;
		FrightenedStartTime = -FrightenedDurationRef;
		TimeSpentInFrightenedSubstate = 0.0;
		NumGhostsConsumed = 0;

		// Post-conditions.
		check(GetEnum(CurrentWorldTimeSeconds) != EChompPlayingSubstateEnum::Frightened);
		check(NumGhostsConsumed == 0);
	}

	// Note: we keep a reference to the LastPlayingSubstate so that we can call a delegate when the substate changes.
	std::tuple<EChompPlayingSubstateEnum, EChompPlayingSubstateEnum> Tick(const double CurrentWorldTimeSeconds)
	{
		// Compute the current substate.
		const auto CurrentSubstate = GetEnum(CurrentWorldTimeSeconds);

		// Grab a reference to LastPlayingSubstate. We're about to overwrite it.
		const auto OldLastPlayingSubstate = LastPlayingSubstate;

		// Update TimeSpentInFrightenedState if we are transitioning out of frightened state.
		if (OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened &&
			CurrentSubstate != EChompPlayingSubstateEnum::Frightened)
		{
			TimeSpentInFrightenedSubstate += FrightenedDurationRef;
			NumGhostsConsumed = 0;
		}

		// Assign LastPlayingSubstate to the current substate.
		LastPlayingSubstate = CurrentSubstate;

		// Post-conditions.
		check(
			OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened
			&& CurrentSubstate != EChompPlayingSubstateEnum::Frightened
			? TimeSpentInFrightenedSubstate >= FrightenedDurationRef
			: true
		);
		check(
			OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened
			&& CurrentSubstate != EChompPlayingSubstateEnum::Frightened
			? NumGhostsConsumed == 0
			: true
		);

		// Return the current substate and reference to old substate to the caller.
		return std::make_tuple(OldLastPlayingSubstate, CurrentSubstate);
	}

	std::tuple<EChompPlayingSubstateEnum, EChompPlayingSubstateEnum> StopPlaying(const double CurrentWorldTimeSeconds)
	{
		// We're switching to the "None" substate.
		constexpr auto CurrentSubstate = EChompPlayingSubstateEnum::None;

		// Grab a reference to LastPlayingSubstate. We're about to overwrite it.
		const auto OldLastPlayingSubstate = LastPlayingSubstate;

		// Update TimeSpentInFrightenedState if we are transitioning out of frightened state.
		if (OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened)
		{
			TimeSpentInFrightenedSubstate += CurrentWorldTimeSeconds - FrightenedStartTime;
			NumGhostsConsumed = 0;
		}

		// Assign LastPlayingSubstate to the current substate.
		LastPlayingSubstate = CurrentSubstate;

		// Post-conditions.
		checkf(
			OldLastPlayingSubstate != CurrentSubstate,
			TEXT("If we are not playing, you should not be calling StopPlaying() a second time.")
		);
		check(
			OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened
			? TimeSpentInFrightenedSubstate >= CurrentWorldTimeSeconds - FrightenedStartTime
			: true
		);
		check(
			OldLastPlayingSubstate == EChompPlayingSubstateEnum::Frightened
			? NumGhostsConsumed == 0
			: true
		);

		// Return the current substate and reference to old substate to the caller.
		return std::make_tuple(OldLastPlayingSubstate, CurrentSubstate);
	}

	void Frighten(const double CurrentWorldTimeSeconds)
	{
		// If we *are* (present tense) in the Frightened substate,
		const auto IsCurrentlyFrightened = GetEnum(CurrentWorldTimeSeconds) == EChompPlayingSubstateEnum::Frightened;
		const auto OldFrightenedStartTime = FrightenedStartTime;
		if (IsCurrentlyFrightened)
		{
			// Add CurrentWorldTimeSeconds - FrightenedStartTime to the TimeSpentInFrightenedSubstate.
			TimeSpentInFrightenedSubstate += CurrentWorldTimeSeconds - OldFrightenedStartTime;
		}

		// Set the new frightened start time.
		FrightenedStartTime = CurrentWorldTimeSeconds;

		// Post-conditions.
		check(
			IsCurrentlyFrightened
			? TimeSpentInFrightenedSubstate >= CurrentWorldTimeSeconds - OldFrightenedStartTime
			: true
		);
	}

	void IncrementNumGhostsConsumed()
	{
		NumGhostsConsumed++;
	}

	// Get the number of ghosts consumed within the current "frightened" substate duration.
	int GetNumGhostsConsumed(double CurrentWorldTimeSeconds) const
	{
		// Pre-conditions.
		check(
			GetEnum(CurrentWorldTimeSeconds) != EChompPlayingSubstateEnum::Frightened
			? NumGhostsConsumed == 0
			: NumGhostsConsumed >= 0
		);

		return NumGhostsConsumed;
	}
};
