#pragma once

#include "Math/IntPoint.h"
#include "GridLocation.h"
#include <vector>

class IGraph
{
public:
	virtual std::vector<GridLocation> Neighbors(GridLocation Id) const = 0;
    virtual double Cost(GridLocation FromNode, GridLocation ToNode) const = 0;
};
