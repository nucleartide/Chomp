/*
 Sample code from https://www.redblobgames.com/pathfinding/a-star/
 Copyright 2014 Red Blob Games <redblobgames@gmail.com>

 Feel free to use this code in your own projects, including commercial projects
 License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
*/

#include "AStar.h"
#include "Debug.h"

// PriorityQueue provides a simple wrapper around std::priority_queue
// so that I only use the API methods I've studied the docs for.
template <typename T, typename priority_t>
struct PriorityQueue
{
    typedef std::pair<priority_t, T> PQElement;

    std::priority_queue<
        PQElement,               // Type of stored element.
        std::vector<PQElement>,  // Type of container used to store elements.
        std::greater<PQElement>> // The elements with greatest priority are ordered first, then we take the last element in this ordering.
        elements;

    // Clear the priority queue's elements.
    inline bool empty() const
    {
        return elements.empty();
    }

    inline void put(T item, priority_t priority)
    {
        // Using emplace avoids the necessity of having to copy/move a std::pair.
        // Instead, the std::pair is constructed within the .emplace() implementation.
        elements.emplace(priority, item);
    }

    T get()
    {
        T best_item = elements.top().second; // Get the top element of the priority queue, and the underlying item within the pair.
        elements.pop();                      // Pop off the top element.
        return best_item;                    // Return the top element.
    }
};

double ManhattanDistanceHeuristic(FGridLocation a, FGridLocation b)
{
    return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
}

// TODO, after lunch:
// [x] write astarsearch example code
// [ ] update level format for less confusion
// [ ] it's the 0,0, 1,1
// [ ] why doesn't astarsearch break? i thought we reach the goal
// [ ] once written, reconstruct the ideal path with a reconstructing function
// [ ] given this reconstructed path, figure out how to make AGhostAIController move along this path
// ---
// [ ] move onto further game impl tasks

template <typename Location>
void AStarSearch(IGraph *Graph,
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
        auto Current = Frontier.get();
        DEBUG_LOG(TEXT("%d, %d"), Current.X, Current.Y);
        if (Current == Goal)
        {
            DEBUG_LOG(TEXT("breaking early"));
            // Then we're done! Break and end the search.
            break;
        }

        for (auto Next : Graph->Neighbors(Current))
        {
            // There is no difference in cost between Pacman maze nodes, so
            // this A* implementation devolves into Dijkstra's algorithm.
            auto NewCost = CostSoFar[Current] + Graph->Cost(Current, Next); // Always zero.

            // If we haven't recorded a cost for this neighbor node,
            // or if the new cost is less than the current cost (never happens with a constant cost),
            if (CostSoFar.find(Next) == CostSoFar.end() || NewCost < CostSoFar[Next])
            {
                // Place neighbor into priority queue.
                // The priority of the neighbor is solely based on the result of the heuristic function.
                // With the default Manhattan distance heuristic, nodes with the smallest Manhattan distance
                // will take priority for exploration.
                double Priority = NewCost + Heuristic(Next, Goal);
                Frontier.put(Next, Priority);

                // Update our maps.
                CostSoFar[Next] = NewCost; // Always zero.
                CameFrom[Next] = Current;  // Provides a link to parent node.
            }
        }
    }
}

template
void AStarSearch(IGraph *Graph,
                 FGridLocation Start,
                 FGridLocation Goal,
                 std::unordered_map<FGridLocation, FGridLocation> &CameFrom,
                 std::unordered_map<FGridLocation, double> &CostSoFar,
                 const std::function<double(FGridLocation, FGridLocation)> &Heuristic);

#if false

#include <iostream>
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

// Jason: A data structure that represents a mathematical graph.
struct SimpleGraph
{
    std::unordered_map<char, std::vector<char>> edges;

    std::vector<char> neighbors(char id)
    {
        return edges[id];
    }
};

struct FGridLocation
{
    int x, y;
};

// Jason: Creating an example graph struct.
SimpleGraph example_graph{{
    {'A', {'B'}},
    {'B', {'C'}},
    {'C', {'B', 'D', 'F'}},
    {'D', {'C', 'E'}},
    {'E', {'F'}},
    {'F', {}},
}};

// Jason: defining the data structure for a square grid.
// i guess this is more properly a rectangular grid, because width and height can be different.
// uses FGridLocation as key for graph nodes
struct SquareGrid
{
    static std::array<FGridLocation, 4> DIRS;

    int width, height;
    std::unordered_set<FGridLocation> walls;

    SquareGrid(int width_, int height_)
        : width(width_), height(height_) {}

    bool in_bounds(FGridLocation id) const
    {
        return 0 <= id.x && id.x < width && 0 <= id.y && id.y < height;
    }

    bool passable(FGridLocation id) const
    {
        return walls.find(id) == walls.end();
    }

    std::vector<FGridLocation> neighbors(FGridLocation id) const
    {
        std::vector<FGridLocation> results;

        for (FGridLocation dir : DIRS)
        {
            FGridLocation next{id.x + dir.x, id.y + dir.y};
            if (in_bounds(next) && passable(next))
            {
                results.push_back(next);
            }
        }

        if ((id.x + id.y) % 2 == 0)
        {
            // see "Ugly paths" section for an explanation:
            std::reverse(results.begin(), results.end());
        }

        return results;
    }
};

// declaring the values for some static variables
std::array<FGridLocation, 4> SquareGrid::DIRS = {
    /* East, West, North, South */
    FGridLocation{1, 0}, FGridLocation{-1, 0},
    FGridLocation{0, -1}, FGridLocation{0, 1}};

// This outputs a grid. Pass in a distances map if you want to print
// the distances, or pass in a point_to map if you want to print
// arrows that point to the parent location, or pass in a path vector
// if you want to draw the path.
//
// jason: could always implement this function by instantiating actors in unreal
template <class Graph>
void draw_grid(const Graph &graph,
               std::unordered_map<FGridLocation, double> *distances = nullptr,
               std::unordered_map<FGridLocation, FGridLocation> *point_to = nullptr,
               std::vector<FGridLocation> *path = nullptr,
               FGridLocation *start = nullptr,
               FGridLocation *goal = nullptr)
{
    const int field_width = 3;
    std::cout << std::string(field_width * graph.width, '_') << '\n';
    for (int y = 0; y != graph.height; ++y)
    {
        for (int x = 0; x != graph.width; ++x)
        {
            FGridLocation id{x, y};
            if (graph.walls.find(id) != graph.walls.end())
            {
                std::cout << std::string(field_width, '#');
            }
            else if (start && id == *start)
            {
                std::cout << " A ";
            }
            else if (goal && id == *goal)
            {
                std::cout << " Z ";
            }
            else if (path != nullptr && find(path->begin(), path->end(), id) != path->end())
            {
                std::cout << " @ ";
            }
            else if (point_to != nullptr && point_to->count(id))
            {
                FGridLocation next = (*point_to)[id];
                if (next.x == x + 1)
                {
                    std::cout << " > ";
                }
                else if (next.x == x - 1)
                {
                    std::cout << " < ";
                }
                else if (next.y == y + 1)
                {
                    std::cout << " v ";
                }
                else if (next.y == y - 1)
                {
                    std::cout << " ^ ";
                }
                else
                {
                    std::cout << " * ";
                }
            }
            else if (distances != nullptr && distances->count(id))
            {
                std::cout << ' ' << std::left << std::setw(field_width - 1) << (*distances)[id];
            }
            else
            {
                std::cout << " . ";
            }
        }
        std::cout << '\n';
    }
    std::cout << std::string(field_width * graph.width, '~') << '\n';
}

// jason: add a rect of walls to a SquareGrid
void add_rect(SquareGrid &grid, int x1, int y1, int x2, int y2)
{
    for (int x = x1; x < x2; ++x)
    {
        for (int y = y1; y < y2; ++y)
        {
            grid.walls.insert(FGridLocation{x, y});
        }
    }
}

// jason: make a test SquareGrid
SquareGrid make_diagram1()
{
    SquareGrid grid(30, 15);
    add_rect(grid, 3, 3, 5, 12);
    add_rect(grid, 13, 4, 15, 15);
    add_rect(grid, 21, 0, 23, 7);
    add_rect(grid, 23, 5, 26, 7);
    return grid;
}

// grid with weights/costs
// from_node isn't used in the cost() method though
struct GridWithWeights : SquareGrid
{
    std::unordered_set<FGridLocation> forests;
    GridWithWeights(int w, int h) : SquareGrid(w, h) {}
    double cost(FGridLocation from_node, FGridLocation to_node) const
    {
        return forests.find(to_node) != forests.end() ? 5 : 1;
    }
};

// jason: make a test GridWithWeights
GridWithWeights make_diagram4()
{
    GridWithWeights grid(10, 10);
    add_rect(grid, 1, 7, 4, 9);
    typedef FGridLocation L;
    grid.forests = std::unordered_set<FGridLocation>{
        L{3, 4}, L{3, 5}, L{4, 1}, L{4, 2},
        L{4, 3}, L{4, 4}, L{4, 5}, L{4, 6},
        L{4, 7}, L{4, 8}, L{5, 1}, L{5, 2},
        L{5, 3}, L{5, 4}, L{5, 5}, L{5, 6},
        L{5, 7}, L{5, 8}, L{6, 2}, L{6, 3},
        L{6, 4}, L{6, 5}, L{6, 6}, L{6, 7},
        L{7, 3}, L{7, 4}, L{7, 5}};
    return grid;
}

// jason: implementation of dijkstra
template <typename Location, typename Graph>
void dijkstra_search(Graph graph,
                     Location start,
                     Location goal,
                     std::unordered_map<Location, Location> &came_from,
                     std::unordered_map<Location, double> &cost_so_far)
{
    PriorityQueue<Location, double> frontier;
    frontier.put(start, 0);

    came_from[start] = start;
    cost_so_far[start] = 0;

    while (!frontier.empty())
    {
        Location current = frontier.get();

        if (current == goal)
        {
            break;
        }

        for (Location next : graph.neighbors(current))
        {
            double new_cost = cost_so_far[current] + graph.cost(current, next);
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
            {
                cost_so_far[next] = new_cost;
                came_from[next] = current;
                frontier.put(next, new_cost);
            }
        }
    }
}

// jason: reconstructing a path given a computed map of preceding nodes on a path
template <typename Location>
std::vector<Location> reconstruct_path(
    Location start, Location goal,
    std::unordered_map<Location, Location> came_from)
{
    std::vector<Location> path;
    Location current = goal;
    if (came_from.find(goal) == came_from.end())
    {
        return path; // no path can be found
    }
    while (current != start)
    {
        path.push_back(current);
        current = came_from[current];
    }
    path.push_back(start); // optional
    std::reverse(path.begin(), path.end());
    return path;
}

// jason: exactly as the function states
GridWithWeights make_diagram_nopath()
{
    GridWithWeights grid(10, 10);
    add_rect(grid, 5, 0, 6, 10);
    return grid;
}

void breadth_first_search(SimpleGraph graph, char start)
{
    std::queue<char> frontier;
    frontier.push(start);

    std::unordered_set<char> reached;
    reached.insert(start);

    while (!frontier.empty())
    {
        char current = frontier.front();
        frontier.pop();

        std::cout << "  Visiting " << current << '\n';
        for (char next : graph.neighbors(current))
        {
            if (reached.find(next) == reached.end())
            {
                frontier.push(next);
                reached.insert(next);
            }
        }
    }
}

// this implementation uses external storage (versus internal storage like a separate Node class)
// the external storage is the unordered_map
template <typename Location, typename Graph>
std::unordered_map<Location, Location>
breadth_first_search(Graph graph, Location start, Location goal)
{
    std::queue<Location> frontier;
    frontier.push(start);

    std::unordered_map<Location, Location> came_from;
    came_from[start] = start;

    while (!frontier.empty())
    {
        Location current = frontier.front();
        frontier.pop();

        // break as soon as we find the goal
        if (current == goal)
        {
            break;
        }

        for (Location next : graph.neighbors(current))
        {
            if (came_from.find(next) == came_from.end())
            {
                frontier.push(next);
                came_from[next] = current;
            }
        }
    }
    return came_from;
}

#endif
