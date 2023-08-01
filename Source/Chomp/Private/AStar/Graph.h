#pragma once

#include "Math/IntPoint.h"
#include <vector>

struct FGridLocation;

class IGraph
{
protected:
	~IGraph() = default;

public:
	virtual std::vector<FGridLocation> Neighbors(FGridLocation Id) const = 0;
    virtual double Cost(FGridLocation FromNode, FGridLocation ToNode) const = 0;
};
