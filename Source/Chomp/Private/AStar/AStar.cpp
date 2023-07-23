/*
 Sample code from https://www.redblobgames.com/pathfinding/a-star/
 Copyright 2014 Red Blob Games <redblobgames@gmail.com>

 Feel free to use this code in your own projects, including commercial projects
 License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
*/

#include "AStar/AStar.h"
#include "Utils/PriorityQueue.h"
#include "LevelGenerator/LevelLoader.h"

double AStar::ManhattanDistanceHeuristic(FGridLocation a, FGridLocation b)
{
    return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
}

template <typename Location>
void AStar::Pathfind(IGraph *Graph,
                     Location Start,
                     Location Goal,
                     std::unordered_map<Location, Location> &CameFrom,
                     std::unordered_map<Location, double> &CostSoFar,
                     const std::function<double(Location, Location)> &Heuristic)
{
    PriorityQueue<Location, double> Frontier;
    Frontier.put(Start, 0);

    CameFrom[Start] = Start;
    CostSoFar[Start] = 0;

    while (!Frontier.empty())
    {
        // If we're at the goal, then we're done! Break and end the search.
        auto Current = Frontier.get();
        if (Current == Goal)
            break;

        // For each neighbor,
        for (auto Next : Graph->Neighbors(Current))
        {
            // Compute the new cost.
            auto NewCost = CostSoFar[Current] + Graph->Cost(Current, Next);

            // If we haven't recorded a cost for this neighbor node, or if the new cost is less than the current cost,
            if (!CostSoFar.contains(Next) || NewCost < CostSoFar[Next])
            {
                // Place neighbor into priority queue.
                double Priority = NewCost + Heuristic(Next, Goal);
                Frontier.put(Next, Priority);

                // Update our maps.
                CostSoFar[Next] = NewCost;
                CameFrom[Next] = Current;
            }
        }
    }
}

template void AStar::Pathfind(IGraph *Graph,
                              FGridLocation Start,
                              FGridLocation Goal,
                              std::unordered_map<FGridLocation, FGridLocation> &CameFrom,
                              std::unordered_map<FGridLocation, double> &CostSoFar,
                              const std::function<double(FGridLocation, FGridLocation)> &Heuristic);

std::vector<FGridLocation> AStar::ReconstructPath(
    const FVector2D CurrentWorldPosition,
    const FGridLocation& Start,
    const FGridLocation& Goal,
    std::unordered_map<FGridLocation, FGridLocation> &CameFrom)
{
    std::vector<FGridLocation> Path;
    FGridLocation Current = Goal;

    // If Goal is not found,
    if (CameFrom.find(Goal) == CameFrom.end())
    {
        // Then no Path could be found.
        return Path;
    }

    // Reconstruct path.
    while (Current != Start)
    {
        Path.push_back(Current);
        Current = CameFrom[Current];
    }
    Path.push_back(Current);

    // Compute some values so that we can snap movement to the grid before moving on the A*-computed path.
    const auto [X2, Y2] = Path[CameFrom.size() - 2];
    const auto [X1, Y1] = Current;
    const auto [SnapX, SnapY] = ULevelLoader::SnapToGridDirection(CurrentWorldPosition);

    if (Y2 == Y1)
    {
        // Ensure horizontal alignment (align on the Y).
        const FGridLocation AlignmentNode{X1, Y1 - SnapY};
        Path.push_back(AlignmentNode);
    }
    else if (X2 == X1)
    {
        // Ensure vertical alignment (align on the X).
        const FGridLocation AlignmentNode{X1 - SnapX, Y1};
        Path.push_back(AlignmentNode);
    }

    // Return the reversed path.
    std::reverse(Path.begin(), Path.end());
    return Path;
}
