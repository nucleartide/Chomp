#include "GridLocation.h"
#include <functional>

bool operator == (GridLocation a, GridLocation b) {
  return a.X == b.X && a.Y == b.Y;
}

bool operator!=(GridLocation a, GridLocation b)
{
    return !(a == b);
}

bool operator<(GridLocation a, GridLocation b)
{
    return std::tie(a.X, a.Y) < std::tie(b.X, b.Y);
}
