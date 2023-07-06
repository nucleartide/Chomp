#pragma once

#include "Math/IntPoint.h"
#include "GridLocation.h"
#include <vector>

class IGraph
{
public:
	virtual std::vector<FGridLocation> Neighbors(FGridLocation Id) const = 0;
    virtual double Cost(FGridLocation FromNode, FGridLocation ToNode) const = 0;
};
