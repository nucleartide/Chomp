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

#include "Math/IntPoint.h"

#include "GridLocation.h"
#include "Graph.h"

double ManhattanDistanceHeuristic(FGridLocation a, FGridLocation b);

template <typename Location>
void AStarSearch(IGraph *Graph,
                 Location Start,
                 Location Goal,
                 std::unordered_map<Location, Location> &CameFrom,
                 std::unordered_map<Location, double> &CostSoFar,
                 const std::function<double(Location, Location)> &Heuristic);

template <typename Location>
std::vector<Location> ReconstructPath(Location Start,
                                      Location Goal,
                                      std::unordered_map<Location, Location> &CameFrom);
