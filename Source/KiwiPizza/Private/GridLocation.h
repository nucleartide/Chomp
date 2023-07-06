#pragma once

struct GridLocation {
  int X, Y;
};

bool operator == (GridLocation a, GridLocation b);
bool operator!=(GridLocation a, GridLocation b);
bool operator<(GridLocation a, GridLocation b);

namespace std
{
    /* implement hash function so we can put GridLocation into an unordered_set */
    template <>
    struct hash<GridLocation>
    {
        std::size_t operator()(const GridLocation &id) const noexcept
        {
            // NOTE: better to use something like boost hash_combine
            return std::hash<int>()(id.X ^ (id.Y << 16));
        }
    };
}
