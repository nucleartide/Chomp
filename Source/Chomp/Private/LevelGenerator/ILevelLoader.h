#pragma once

struct FGridLocation;

class ILevelLoader
{
protected:
	~ILevelLoader() = default;
	
public:
	virtual FVector2D GridToWorld(const FGridLocation& GridPosition) const = 0;
	virtual bool IsWall(const FGridLocation& Location) const = 0;
};
