#pragma once

#include "Graph.h"
#include "Math/IntPoint.h"
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
#include <algorithm>
#include <cstdlib>
#include "GridLocation.h"

double ManhattanDistanceHeuristic(GridLocation a, GridLocation b);

template <typename Location>
void AStarSearch(IGraph *Graph,
                 Location Start,
                 Location Goal,
                 std::unordered_map<Location, Location> &CameFrom,
                 std::unordered_map<Location, double> &CostSoFar,
                 const std::function<double(Location, Location)> &Heuristic);
