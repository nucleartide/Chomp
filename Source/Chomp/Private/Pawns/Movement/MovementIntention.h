#pragma once

#include "Movement.h"
#include "AStar/GridLocation.h"

struct FMovementIntention
{
	const FGridLocation Direction;
	const double TimeOfLastUpdate;

	FMovementIntention(
		const float VerticalAxisInput,
		const float HorizontalAxisInput,
		const UWorld* WorldInstance
	) :
		Direction(FGridLocation{
			FGenericPlatformMath::RoundToInt(VerticalAxisInput),
			FGenericPlatformMath::RoundToInt(HorizontalAxisInput)
		}),
		TimeOfLastUpdate(WorldInstance->GetRealTimeSeconds())
	{
	}

	FMovementIntention(
		const FGridLocation& Direction,
		const UWorld* WorldInstance
	) :
		Direction(Direction),
		TimeOfLastUpdate(WorldInstance->GetRealTimeSeconds())
	{
	}

	bool HasElapsedSinceLastUpdate(const double Duration, const UWorld* WorldInstance) const
	{
		return WorldInstance->GetRealTimeSeconds() > TimeOfLastUpdate + Duration;
	}
};
