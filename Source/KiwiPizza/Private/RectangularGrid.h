#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include <unordered_set>
#include <array>
#include <vector>

using GridLocation = FIntPoint;

namespace std
{
	/* implement hash function so we can put GridLocation into an unordered_set */
	template <>
	struct hash<GridLocation>
	{
		std::size_t operator()(const GridLocation &id) const noexcept
		{
			// NOTE: better to use something like boost hash_combine
			return std::hash<int>()(id.X ^ (id.Y << 16));
		}
	};
}

// RectangularGrid is a graph where GridLocation is used as the key for graph nodes.
//
// [ ] TODO: Add an instance of this to LevelLoader.
struct RectangularGrid
{
public:
	// (GridLocation is a struct containing X and Y.)
	static std::array<GridLocation, 4> DIRS;

	int Width;

	int Height;

	std::unordered_set<GridLocation> Walls;

	RectangularGrid(int _Width, int _Height)
		: Width(_Width), Height(_Height) {}

	bool InBounds(GridLocation Id) const;

	bool Passable(GridLocation Id) const;

	std::vector<GridLocation> Neighbors(GridLocation Id) const;
};
