/*
 Sample code from https://www.redblobgames.com/pathfinding/a-star/
 Copyright 2014 Red Blob Games <redblobgames@gmail.com>

 Feel free to use this code in your own projects, including commercial projects
 License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
*/

#include "AStar/AStar.h"
#include "Utils/PriorityQueue.h"
#include "LevelGenerator/LevelLoader.h"

double FAStar::ManhattanDistanceHeuristic(const FGridLocation& A, const FGridLocation& B)
{
    return std::abs(A.X - B.X) + std::abs(A.Y - B.Y);
}

template <typename Location>
void FAStar::Pathfind(IGraph *Graph,
                     Location Start,
                     Location Goal,
                     std::unordered_map<Location, Location> &CameFrom,
                     std::unordered_map<Location, double> &CostSoFar,
                     const std::function<double(Location, Location)> &Heuristic)
{
    TPriorityQueue<Location, double> Frontier;
    Frontier.Put(Start, 0);

    CameFrom[Start] = Start;
    CostSoFar[Start] = 0;

    while (!Frontier.Empty())
    {
        // If we're at the goal, then we're done! Break and end the search.
        auto Current = Frontier.Get();
        if (Current == Goal)
            break;

        // For each neighbor,
        for (auto Next : Graph->Neighbors(Current))
        {
            // Compute the new cost.
            auto NewCost = CostSoFar[Current] + Graph->Cost(Current, Next);

            // If we haven't recorded a cost for this neighbor node, or if the new cost is less than the current cost,
            // ReSharper disable once CppUseAssociativeContains
            if (CostSoFar.find(Next) == CostSoFar.end() || NewCost < CostSoFar[Next])
            {
                // Place neighbor into priority queue.
                double Priority = NewCost + Heuristic(Next, Goal);
                Frontier.Put(Next, Priority);

                // Update our maps.
                CostSoFar[Next] = NewCost;
                CameFrom[Next] = Current;
            }
        }
    }
}

template void FAStar::Pathfind(IGraph *Graph,
                              FGridLocation Start,
                              FGridLocation Goal,
                              std::unordered_map<FGridLocation, FGridLocation> &CameFrom,
                              std::unordered_map<FGridLocation, double> &CostSoFar,
                              const std::function<double(FGridLocation, FGridLocation)> &Heuristic);

std::vector<FGridLocation> FAStar::ReconstructPath(
    const FGridLocation& Start,
    const FGridLocation& Goal,
    std::unordered_map<FGridLocation, FGridLocation> &CameFrom)
{
    std::vector<FGridLocation> Path;
    FGridLocation Current = Goal;

    // If Goal cannot be found,
    if (CameFrom.find(Goal) == CameFrom.end())
    {
        // Then there is no Path.
        return Path;
    }

    // Reconstruct path.
    while (Current != Start)
    {
        Path.push_back(Current);
        Current = CameFrom[Current];
    }
    Path.push_back(Current);

    // Return the reversed path.
    std::reverse(Path.begin(), Path.end());
    return Path;
}
