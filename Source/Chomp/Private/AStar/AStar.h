#pragma once

#include <functional>
#include <unordered_set>
#include <vector>
#include <utility>
#include <tuple>
#include <cstdlib>
#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "AStar/GridLocation.h"
#include "AStar/Graph.h"

class FAStar
{
public:
	static double ManhattanDistanceHeuristic(const FGridLocation& A, const FGridLocation& B);

	template <typename Location>
	static void Pathfind(IGraph *Graph,
						 Location Start,
						 Location Goal,
						 std::unordered_map<Location, Location> &CameFrom,
						 std::unordered_map<Location, double> &CostSoFar,
						 const std::function<double(Location, Location)> &Heuristic);

	static std::vector<FGridLocation> ReconstructPath(const FGridLocation& Start,
													  const FGridLocation& Goal,
													  std::unordered_map<FGridLocation, FGridLocation> &CameFrom);
};
