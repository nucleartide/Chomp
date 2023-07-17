#pragma once

#include <algorithm>
#include <iostream>
#include <functional>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>
#include <utility>
#include <queue>
#include <tuple>
#include <cstdlib>

#include "CoreMinimal.h"
#include "Math/IntPoint.h"

#include "AStar/GridLocation.h"
#include "AStar/Graph.h"

class AStar
{
public:
	static double ManhattanDistanceHeuristic(FGridLocation a, FGridLocation b);

	template <typename Location>
	static void Pathfind(IGraph *Graph,
						 Location Start,
						 Location Goal,
						 std::unordered_map<Location, Location> &CameFrom,
						 std::unordered_map<Location, double> &CostSoFar,
						 const std::function<double(Location, Location)> &Heuristic);

	static std::vector<FGridLocation> ReconstructPath(FVector2D CurrentWorldPosition,
													  FGridLocation Start,
													  FGridLocation Goal,
													  std::unordered_map<FGridLocation, FGridLocation> &CameFrom);
};
