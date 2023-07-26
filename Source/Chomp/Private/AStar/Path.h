#pragma once

#include <vector>
#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"

struct FPath
{
private:
	// A value of -1 indicates that the Pawn must reach the first point on the path before continuing.
	int CurrentLocationIndex = -1;
	std::vector<FGridLocation> Locations;

public:
	explicit FPath()
	{
		// Default values are fine. This would just be a zero-node path.
	}
	
	explicit FPath(
		const FVector& CurrentWorldLocation,
		const std::vector<FGridLocation>& LocationPath,
		const ULevelLoader *LevelInstance)
	{
		// Update CurrentLocationIndex.
		for (auto i = 0; i < LocationPath.size() - 1; i++)
		{
			auto CurrentNode = LocationPath[i];
			auto NextNode = LocationPath[i + 1];
			if (FGridLocation::IsInBetween(CurrentWorldLocation, CurrentNode, NextNode, LevelInstance))
				CurrentLocationIndex = i;
		}

		// Sanity check on CurrentLocationIndex.
		// If we haven't reached the start node yet,
		if (CurrentLocationIndex == -1 && LocationPath.size() >= 1)
		{
			// Then CurrentWorldLocation must be within 50 axis-aligned units (inclusive) of the start node.
			const auto WorldA = LevelInstance->GridToWorld(LocationPath.at(0));
			check(
				CurrentWorldLocation.X == WorldA.X && FGenericPlatformMath::Abs(CurrentWorldLocation.Y - WorldA.Y) <= 50.0f ||
				CurrentWorldLocation.Y == WorldA.Y && FGenericPlatformMath::Abs(CurrentWorldLocation.X - WorldA.X) <= 50.0f);
		}

		// Sanity check on LocationPath.
		// Assert that each grid location is not a wall.
		for (auto Location : LocationPath)
		{
			check(!LevelInstance->IsWall(Location));
		}

		// Update Locations.
		Locations = LocationPath;
	}

	// [ ] refactor ai controller so that you no longer have target tile nor current direction

	void NextNode()
	{
		CurrentLocationIndex++;
	}

#if false
	// TODO.
	FGridLocation GetCurrentMoveDirection(FVector WorldLocation, const ULevelLoader* LevelInstance)
	{
		if (CurrentLocationIndex == -1)
		{
			const auto StartGridPos = Locations.at(0);
			const auto StartWorldPos = LevelInstance->GridToWorld(StartGridPos);
			const auto DirX = StartWorldPos.X - WorldLocation.X;
			const auto DirY = StartWorldPos.Y - WorldLocation.Y;
			const auto SignX = DirX < 0.0 ? -1 : DirX > 0.0 ? 1 : 0;
			const auto SignY = DirY < 0.0 ? -1 : DirY > 0.0 ? 1 : 0;
			// Both can't be 1 at the same time, that'd be a diagonal.
			check(!(FMath::Abs(SignX) == 1 && FMath::Abs(SignY) == 1));
			FGridLocation Result{SignX, SignY};
			return Result;
		}

		if (const int NumLocations = Locations.size(); CurrentLocationIndex > NumLocations - 2)
		{
			FGridLocation Zero{0, 0};
			return Zero;
		}

		const auto [CurrentX, CurrentY] = Locations.at(CurrentLocationIndex);
		const auto [NextX, NextY] = Locations.at(CurrentLocationIndex + 1);
		FGridLocation Result{NextX - CurrentX, NextY - CurrentY};
		check(FMath::Abs(Result.X) <= 1);
		check(FMath::Abs(Result.Y) <= 1);
		return Result;
	}
#endif

	// Unit test this when I have time: zero location path should also work.
	bool WasCompleted(int Index = -1) const
	{
		if (Index == -1)
			Index = Locations.size() - 2;

		// Example:
		// 0, 1, 2, 3, 4
		// if CurrentLocationIndex == 4 or above, the path was completed.
		return CurrentLocationIndex > Index;
	}

	FGridLocation GetTargetLocation() const
	{
		return Locations.at(CurrentLocationIndex + 1);
	}

	void DebugLog(const FString Label) const
	{
		FString DynamicString{""};
		auto DateTime = FDateTime::Now().ToString();
		DynamicString += TEXT("[");
		DynamicString += DateTime;
		DynamicString += TEXT("] ");
		DynamicString += Label;
		DynamicString += TEXT(" Locations: ");
		for (auto Location : Locations)
			DynamicString += FString::Printf(TEXT("%s, "), *Location.ToString());
		DEBUG_LOG(TEXT("%s"), *DynamicString);
	}
};
