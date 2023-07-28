#pragma once

#include "AStar/GridLocation.h"

struct FMovement
{
	const FGridLocation Direction;
	const FMaybeGridLocation TargetTile; // Not sure if I need this yet.

	explicit FMovement(
		const FGridLocation& Direction,
		const FMaybeGridLocation& TargetTile
	) :
		Direction(Direction),
		TargetTile(TargetTile)
	{
		if (TargetTile.IsValid)
			check(Direction.IsNonZero());
	}

	bool HasValidTargetTile() const
	{
		return TargetTile.IsValid;
	}
};
