#pragma once

#include "AStar/GridLocation.h"

#include "Movement.generated.h"

USTRUCT()
struct FMovement
{
	GENERATED_BODY()

private:
	FGridLocation Direction;
	FMaybeGridLocation TargetTile;

public:
	FMovement(): Direction(FGridLocation{0, 0}), TargetTile(FMaybeGridLocation{false, FGridLocation{0, 0}})
	{
	}

	explicit FMovement(
		const FGridLocation& Direction,
		const FMaybeGridLocation& TargetTile
	) :
		Direction(Direction),
		TargetTile(TargetTile)
	{
		check(!TargetTile.IsValid || HasValidTargetTile());
	}

	FMovement& operator=(const FMovement& Other)
	{
		if (this == &Other)
			return *this;
		Direction = Other.Direction;
		TargetTile = Other.TargetTile;
		return *this;
	}

	bool HasValidTargetTile() const
	{
		return TargetTile.IsValid && Direction.IsNonZero();
	}

	[[nodiscard]] FGridLocation GetDirection() const
	{
		return Direction;
	}

	[[nodiscard]] FMaybeGridLocation GetTargetTile() const
	{
		return TargetTile;
	}

	void Reset()
	{
		Direction = FGridLocation{0, 0};
		TargetTile = FMaybeGridLocation{false, FGridLocation{0, 0}};
	}
};
