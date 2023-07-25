#pragma once
#include <vector>

#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"

struct FPath
{
private:
	// Should be initialized to -1 so that Pawn can reach the first point on the path before continuing.
	int CurrentLocationIndex = -1;
	std::vector<FGridLocation> Locations;

public:
	explicit FPath()
	{
		CurrentLocationIndex = -1;
	}

	explicit FPath(const std::vector<FGridLocation>& NewLocations)
	{
		CurrentLocationIndex = -1;
		Locations = NewLocations;
	}

	void Increment()
	{
		CurrentLocationIndex++;
	}

	int GetCurrentLocationIndex() const
	{
		return CurrentLocationIndex;
	}

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

		const int NumLocations = Locations.size();
		if (CurrentLocationIndex > NumLocations - 2)
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
