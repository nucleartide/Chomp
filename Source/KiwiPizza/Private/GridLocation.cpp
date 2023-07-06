#include "GridLocation.h"
#include <functional>

bool operator == (FGridLocation a, FGridLocation b) {
  return a.X == b.X && a.Y == b.Y;
}

bool operator!=(FGridLocation a, FGridLocation b)
{
    return !(a == b);
}

bool operator<(FGridLocation a, FGridLocation b)
{
    return std::tie(a.X, a.Y) < std::tie(b.X, b.Y);
}
