#pragma once

#include "AStar/GridLocation.h"

struct FMovement
{
	const FGridLocation Direction;
	const FMaybeGridLocation TargetTile;

	explicit FMovement(
		const FGridLocation& Direction,
		const FMaybeGridLocation& TargetTile
	) :
		Direction(Direction),
		TargetTile(TargetTile)
	{
		check(!TargetTile.IsValid || TargetTile.IsValid && Direction.IsNonZero());
	}

	bool HasValidTargetTile() const
	{
		return TargetTile.IsValid && Direction.IsNonZero();
	}
};
