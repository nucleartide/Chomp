#pragma once

#include <tuple>

#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"

#include "MovementPath.generated.h"

USTRUCT()
struct FMovementPath
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	TArray<FGridLocation> GridLocationPath;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> WorldLocationPath;

	UPROPERTY(VisibleAnywhere)
	const ULevelLoader* LevelInstance;

	// bool: Whether ActorLocation is on path.
	// int: The index of the next GridLocation. Will be -1 if there is no next GridLocation.
	static std::tuple<bool, int> IsOnPath(
		const FVector& ActorLocation,
		const TArray<FGridLocation>& Path,
		const ULevelLoader* LevelInstance)
	{
		// Empty path case.
		if (Path.Num() == 0)
			return std::make_tuple(false, -1);

		// Case where ActorLocation is on path.
		const FVector2D ActorLocation2D(ActorLocation);
		for (auto i = 0; i < Path.Num() - 1; i++)
		{
			auto CurrentNode = Path[i];
			auto NextNode = Path[i + 1];
			if (FGridLocation::IsInBetween(ActorLocation, CurrentNode, NextNode, LevelInstance))
				return std::make_tuple(true, i + 1);
		}

		// If ActorLocation is within 50 axis-aligned units (inclusive) of the start node,
		const auto WorldA = LevelInstance->GridToWorld(Path[0]);
		if (FMath::IsNearlyEqual(ActorLocation.X, WorldA.X) &&
			FGenericPlatformMath::Abs(ActorLocation.Y - WorldA.Y) <= 50.0 ||
			FMath::IsNearlyEqual(ActorLocation.Y, WorldA.Y) &&
			FGenericPlatformMath::Abs(ActorLocation.X - WorldA.X) <= 50.0)
			return std::make_tuple(true, 0);

		// If ActorLocation is at the end node,
		const auto WorldLast = LevelInstance->GridToWorld(Path[Path.Num() - 1]);
		if (FMath::IsNearlyEqual(ActorLocation.X, WorldLast.X) &&
			FMath::IsNearlyEqual(ActorLocation.Y, WorldLast.Y))
			return std::make_tuple(true, -1);

		return std::make_tuple(false, -1);
	}

public:
	FMovementPath(): LevelInstance(nullptr)
	{
	}

	explicit FMovementPath(
		const FVector& ActorLocation,
		const TArray<FGridLocation>& NewLocationPath,
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
			WorldLocationPath.Add(FVector{WorldLocation.X, WorldLocation.Y, 0.0f});
		}
	}

	bool WasCompleted(const FVector& ActorLocation) const
	{
		if (GridLocationPath.Num() == 0)
			return true;

		const auto LastGridLocation = GridLocationPath[GridLocationPath.Num() - 1];
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
		// TODO: replace with a Lerp implementation.
		// ...
		
		// If we move 100+ cm, we have a problem because our algorithm is designed
		// to work for a DeltaDistance of less than 100 cm.
		//
		// In this case, don't move at all.
		if (DeltaDistance >= 100.0)
			return ActorLocation;

		// Compute direction.
		FVector DestWorldLocation;
		FVector Direction{0.0, 0.0, 0.0};
		{
			const auto [OnPath, DestIndex] = IsOnPath(ActorLocation, GridLocationPath, LevelInstance);
			if (!OnPath || DestIndex == -1)
				return ActorLocation;

			DestWorldLocation = WorldLocationPath[DestIndex];
			Direction = (DestWorldLocation - ActorLocation).GetUnsafeNormal();
			check(
				!Direction.IsNearlyZero() &&
				FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Direction.X), 1.0) &&
				FMath::IsNearlyEqual(Direction.Y, 0.0) ||
				FMath::IsNearlyEqual(Direction.X, 0.0) &&
				FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Direction.Y), 1.0)
			);
		}

		// Apply movement.
		ActorLocation += DeltaDistance * Direction;

		// Check if we overshot.
		// We MovedPastTarget if MovementDotProduct is around -1.0.
		const FVector2D Direction2D(Direction);
		const FVector2D DestWorldLocation2D(DestWorldLocation);
		const FVector2D ActorLocation2D(ActorLocation);
		const auto MovementDotProduct = FVector::DotProduct(
			Direction,
			(DestWorldLocation - ActorLocation).GetUnsafeNormal());
		const auto AmountDotProduct = FVector2D::DotProduct(
			Direction2D,
			DestWorldLocation2D - ActorLocation2D);
		const auto MovedPastTarget = FMath::Abs(MovementDotProduct + 1.0) < 0.1;
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
				const auto NextDest = WorldLocationPath[DestIndex];
				const auto NextDir = (NextDest - ActorLocation).GetUnsafeNormal();

				// And apply the remaining movement.
				ActorLocation += AmountMovedPast * NextDir;
			}
		}

		// Finally, return the computed ActorLocation.
		return ActorLocation;
	}

	FMovementPath& operator=(const FMovementPath& Other)
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
		const auto GridLocationPathSize = GridLocationPath.Num();
		check(GridLocationPathSize == WorldLocationPath.Num());
		return GridLocationPathSize > 0;
	}

	FMaybeGridLocation GetEndNode() const
	{
		const auto Path = GridLocationPath;
		if (Path.Num() == 0)
			return FMaybeGridLocation::Invalid();

		return FMaybeGridLocation::Valid(Path[Path.Num() - 1]);
	}
};
