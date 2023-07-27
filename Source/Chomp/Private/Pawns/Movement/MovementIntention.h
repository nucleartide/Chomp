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
		check(VerticalAxisInput != 0.0f || HorizontalAxisInput != 0.0f);
	}

	FMovementIntention(
		const FGridLocation& Direction,
		const UWorld* WorldInstance
	) :
		Direction(Direction),
		TimeOfLastUpdate(WorldInstance->GetRealTimeSeconds())
	{
		check(Direction.IsNonZero());
	}

	bool SinceLastUpdate(const double Duration, const UWorld* WorldInstance) const
	{
		return WorldInstance->GetRealTimeSeconds() > TimeOfLastUpdate + Duration;
	}

	bool IsDifferentFrom(const TSharedPtr<FMovement>& CurrentMovement) const
	{
		const auto CurrentDirection = CurrentMovement->Direction;
		return CurrentDirection.IsNonZero() && CurrentDirection != Direction;
	}
};
