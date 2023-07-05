#include "RectangularGrid.h"

bool RectangularGrid::InBounds(GridLocation Id) const
{
    return 0 <= Id.X && Id.X < Width && 0 <= Id.Y && Id.Y < Height;
}

bool RectangularGrid::Passable(GridLocation Id) const
{
    return Walls.find(Id) == Walls.end();
}

// Get the neighbors of a GridLocation.
std::vector<GridLocation> RectangularGrid::Neighbors(GridLocation Id) const
{
    std::vector<GridLocation> Results;

    for (GridLocation Direction : DIRS)
    {
        GridLocation Next{Id.X + Direction.X, Id.Y + Direction.Y};
        if (InBounds(Next) && Passable(Next))
            Results.push_back(Next);
    }

    // Not 100% sure why we need this, but see this link for explanations: https://www.redblobgames.com/pathfinding/a-star/implementation.html#troubleshooting-ugly-path
    if ((Id.X + Id.Y) % 2 == 0)
    {
        // Reverse the list of neighbors.
        std::reverse(Results.begin(), Results.end());
    }

    return Results;
}
