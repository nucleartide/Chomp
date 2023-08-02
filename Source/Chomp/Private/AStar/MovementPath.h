#pragma once

#include <vector>
#include <tuple>

#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"

#include "MovementPath.generated.h"

UCLASS()
class UMovementPath : public UObject
{
	GENERATED_BODY()

	std::vector<FGridLocation> GridLocationPath;
	std::vector<FVector> WorldLocationPath;

	UPROPERTY()
	const ULevelLoader* LevelInstance;

	// bool: Whether ActorLocation is on path.
	// int: The index of the next GridLocation. Will be -1 if there is no next GridLocation.
	static std::tuple<bool, int> IsOnPath(
		const FVector& ActorLocation,
		const std::vector<FGridLocation>& Path,
		const ULevelLoader* LevelInstance)
	{
		if (Path.size() == 0)
			return std::make_tuple(false, -1);

		// If ActorLocation is on the path somewhere,
		const FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
		for (auto i = 0; i < Path.size() - 1; i++)
		{
			auto CurrentNode = Path[i];
			auto NextNode = Path[i + 1];
			if (FGridLocation::IsInBetween(ActorLocation, CurrentNode, NextNode, LevelInstance))
				return std::make_tuple(true, i + 1);
		}

		// If ActorLocation is within 50 axis-aligned units (inclusive) of the start node,
		const auto WorldA = LevelInstance->GridToWorld(Path.at(0));
		if (FMath::IsNearlyEqual(ActorLocation.X, WorldA.X, 0.01f) &&
			FGenericPlatformMath::Abs(ActorLocation.Y - WorldA.Y) <= 50.0f ||
			FMath::IsNearlyEqual(ActorLocation.Y, WorldA.Y, 0.01f) &&
			FGenericPlatformMath::Abs(ActorLocation.X - WorldA.X) <= 50.0f)
			return std::make_tuple(true, 0);

		// If ActorLocation is at the end node,
		const auto WorldLast = LevelInstance->GridToWorld(Path.at(Path.size() - 1));
		if (FMath::IsNearlyEqual(ActorLocation.X, WorldLast.X, 0.01f) &&
			FMath::IsNearlyEqual(ActorLocation.Y, WorldLast.Y, 0.01f))
			return std::make_tuple(true, -1);

		return std::make_tuple(false, -1);
	}

public:
	explicit UMovementPath(
		const FVector& ActorLocation,
		const std::vector<FGridLocation>& NewLocationPath,
		const ULevelLoader* LevelInstance) : LevelInstance(LevelInstance)
	{
		// Sanity check.
		check(std::get<0>(IsOnPath(ActorLocation, NewLocationPath, LevelInstance)));
		for (auto Location : NewLocationPath)
			check(!LevelInstance->IsWall(Location));

		// Initialize state.
		GridLocationPath = NewLocationPath;
		for (auto Location : NewLocationPath)
		{
			const auto WorldLocation = LevelInstance->GridToWorld(Location);
			WorldLocationPath.push_back(FVector{WorldLocation.X, WorldLocation.Y, 0.0f});
		}
	}

	bool WasCompleted(const FVector& ActorLocation) const
	{
		if (GridLocationPath.size() == 0)
			return true;

		const auto LastGridLocation = GridLocationPath.at(GridLocationPath.size() - 1);
		const auto LastWorldLoc = LevelInstance->GridToWorld(LastGridLocation);
		return ActorLocation == FVector{LastWorldLoc.X, LastWorldLoc.Y, 0.0f};
	}

	bool DidComplete(const FVector& ActorLocation, int Index) const
	{
		const auto [OnPath, NextIndex] = IsOnPath(ActorLocation, GridLocationPath, LevelInstance);
		return OnPath && (NextIndex == -1 || NextIndex > Index);
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
		for (auto Location : GridLocationPath)
			DynamicString += FString::Printf(TEXT("%s, "), *Location.ToString());
		DEBUG_LOG(TEXT("%s"), *DynamicString);
	}

	FVector MoveAlongPath(FVector ActorLocation, float DeltaDistance) const
	{
		FVector Direction{0.0, 0.0, 0.0};
		FVector DestWorldLocation;
		{
			// Sanity check.
			const auto [OnPath, DestIndex] = IsOnPath(ActorLocation, GridLocationPath, LevelInstance);
			check(OnPath);

			// No movement if there is no destination node.
			if (DestIndex == -1)
				return ActorLocation;

			// Compute direction.
			DestWorldLocation = WorldLocationPath.at(DestIndex);
			Direction = (DestWorldLocation - ActorLocation).GetSafeNormal();
			check(
				FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Direction.X), 1.0) &&
				FMath::IsNearlyEqual(Direction.Y, 0.0) ||
				FMath::IsNearlyEqual(Direction.X, 0.0) &&
				FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Direction.Y), 1.0)
			);
		}

		// Apply movement.
		ActorLocation += DeltaDistance * Direction;

		// Check if we overshot.
		// We MovedPastTarget if MovementDotProduct is around -1.0f.
		FVector2D Direction2D{Direction.X, Direction.Y};
		FVector2D DestWorldLocation2D{DestWorldLocation.X, DestWorldLocation.Y};
		FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
		const auto MovementDotProduct = FVector2D::DotProduct(
			Direction2D,
			(DestWorldLocation2D - ActorLocation2D).GetSafeNormal());
		const auto AmountDotProduct = FVector2D::DotProduct(
			Direction2D,
			DestWorldLocation2D - ActorLocation2D);
		const auto MovedPastTarget = FMath::Abs(MovementDotProduct + 1.0f) < 0.1f;
		const auto AmountMovedPast = FMath::Abs(AmountDotProduct);

		// If we overshot,
		if (MovedPastTarget)
		{
			// Reset the actor location.
			ActorLocation = DestWorldLocation;

			// Check whether there is a next destination node.
			const auto [OnPath, DestIndex] = IsOnPath(ActorLocation, GridLocationPath, LevelInstance);

			// If there is,
			if (OnPath && DestIndex > -1)
			{
				// Compute next direction.
				const auto NextDest = WorldLocationPath.at(DestIndex);
				const auto NextDir = (NextDest - ActorLocation).GetSafeNormal();

				// And apply the remaining movement.
				ActorLocation += AmountMovedPast * NextDir;
			}
		}

		// Finally, return the computed ActorLocation.
		return ActorLocation;
	}

	UMovementPath& operator=(const UMovementPath& Other)
	{
		if (this == &Other)
			return *this;
		GridLocationPath = Other.GridLocationPath;
		WorldLocationPath = Other.WorldLocationPath;
		LevelInstance = Other.LevelInstance;
		return *this;
	}

	bool IsValid() const
	{
		const auto GridLocationPathSize = GridLocationPath.size();
		check(GridLocationPathSize == WorldLocationPath.size());
		return GridLocationPathSize > 0;
	}
};
