#include "AStar/GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include <algorithm>

bool operator ==(const FGridLocation& A, const FGridLocation& B)
{
	return A.X == B.X && A.Y == B.Y;
}

bool operator!=(const FGridLocation& A, const FGridLocation& B)
{
	return !(A == B);
}

bool operator<(const FGridLocation& A, const FGridLocation& B)
{
	return std::tie(A.X, A.Y) < std::tie(B.X, B.Y);
}

FString FGridLocation::ToString() const
{
	return FString::Printf(TEXT("(%d, %d)"), X, Y);
}

bool FGridLocation::IsNonZero() const
{
	return !IsZero();
}

bool FGridLocation::IsZero() const
{
	return X == 0 && Y == 0;
}

bool FGridLocation::IsInBetween(const FVector& Location, const FGridLocation& A, const FGridLocation& B,
                                const ULevelLoader* LevelInstance)
{
	const auto WorldA = LevelInstance->GridToWorld(A);
	const auto WorldB = LevelInstance->GridToWorld(B);

	if (WorldA.X == WorldB.X && Location.X == WorldA.X)
	{
		const auto MinY = std::min(WorldA.Y, WorldB.Y);
		const auto MaxY = std::max(WorldA.Y, WorldB.Y);
		if (MinY <= Location.Y && Location.Y <= MaxY)
			return true;
	}

	if (WorldA.Y == WorldB.Y && Location.Y == WorldA.Y)
	{
		const auto MinX = std::min(WorldA.X, WorldB.X);
		const auto MaxX = std::max(WorldA.X, WorldB.X);
		if (MinX <= Location.X && Location.X <= MaxX)
			return true;
	}

	return false;
}
