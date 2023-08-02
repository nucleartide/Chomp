#pragma once

#include "AStar/GridLocation.h"

#include "MovementIntention.generated.h"

USTRUCT()
struct FMovementIntention
{
	GENERATED_BODY()

private:
	FGridLocation Direction;
	double TimeOfLastUpdate;

public:
	FMovementIntention(): Direction(FGridLocation{0, 0}), TimeOfLastUpdate(0)
	{
	}

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

	FMovementIntention& operator=(const FMovementIntention& Other)
	{
		if (this == &Other)
			return *this;
		Direction = Other.Direction;
		TimeOfLastUpdate = Other.TimeOfLastUpdate;
		return *this;
	}

	bool HasElapsedSinceLastUpdate(const double Duration, const UWorld* WorldInstance) const
	{
		return WorldInstance->GetRealTimeSeconds() > TimeOfLastUpdate + Duration;
	}

	[[nodiscard]] FGridLocation GetDirection() const
	{
		return Direction;
	}

	[[nodiscard]] double GetTimeOfLastUpdate() const
	{
		return TimeOfLastUpdate;
	}

	void Reset()
	{
		Direction = FGridLocation{0, 0};
		TimeOfLastUpdate = 0.0;
	}
};
