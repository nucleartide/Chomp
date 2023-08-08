#pragma once

#include <tuple>

#include "GenericPlatform/GenericPlatformMath.h"
#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"
#include "Utils/MathHelpers.h"

#include "MovementPath.generated.h"

USTRUCT()
struct FMovementPath
{
	GENERATED_BODY()

private:
	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	UPROPERTY(VisibleAnywhere)
	TArray<FGridLocation> GridLocationPath;

	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> WorldLocationPath;

	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	UPROPERTY(VisibleAnywhere)
	const ULevelLoader* LevelInstance;

	static std::optional<double> GetCurrentPathLocation(const FVector& ActorLocation, TArray<FVector> WorldLocationPath)
	{
		// Establish where the actor is along the path as a single value.
		auto CurrentPathLocation = 0.0;

		// Traverse the path to see where the actor is currently on the path.
		const FVector2D ActorLocation2D(ActorLocation);
		for (auto i = 0; i < WorldLocationPath.Num() - 1; i++)
		{
			auto CurrentNode = WorldLocationPath[i];
			auto NextNode = WorldLocationPath[i + 1];

			if (const auto Result = FGridLocation::IsInBetween(ActorLocation, CurrentNode, NextNode);
				Result.has_value())
			{
				CurrentPathLocation += Result.value();
				return CurrentPathLocation;
			}

			CurrentPathLocation += 100.0;
		}

		// If ActorLocation is at the end, then return the end PathLocation value.
		if (ActorLocation.Equals(WorldLocationPath[WorldLocationPath.Num() - 1]))
			return (WorldLocationPath.Num() - 1) * 100.0;

		// However, if we reached the end of the path and the actor was not found,
		// check to see if the actor is actually within 50 units (inclusive) of the start node.
		if (const auto WorldA = WorldLocationPath[0];
			FMath::IsNearlyEqual(ActorLocation.X, WorldA.X) &&
			FGenericPlatformMath::Abs(ActorLocation.Y - WorldA.Y) <= 50.0)
		{
			if (ActorLocation.Y <= WorldA.Y)
				return ActorLocation.Y - WorldA.Y;
			if (WorldA.Y < ActorLocation.Y)
				return WorldA.Y - ActorLocation.Y;
		}
		else if (
			FMath::IsNearlyEqual(ActorLocation.Y, WorldA.Y) &&
			FGenericPlatformMath::Abs(ActorLocation.X - WorldA.X) <= 50.0)
		{
			if (ActorLocation.X <= WorldA.X)
				return ActorLocation.X - WorldA.X;
			if (WorldA.X < ActorLocation.X)
				return WorldA.X - ActorLocation.X;
		}

		return std::nullopt;
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
		// Initialize state.
		GridLocationPath = NewLocationPath;
		for (auto Location : NewLocationPath)
		{
			const auto WorldLocation = LevelInstance->GridToWorld(Location);
			WorldLocationPath.Add(FVector{WorldLocation.X, WorldLocation.Y, 0.0f});
		}

		// Sanity check.
		check(GetCurrentPathLocation(ActorLocation, WorldLocationPath).has_value());
		for (auto Location : NewLocationPath)
			check(!LevelInstance->IsWall(Location));
	}

	bool WasCompleted(const FVector& ActorLocation) const
	{
		if (GridLocationPath.Num() == 0)
			return true;

		const auto LastWorldLoc = WorldLocationPath[WorldLocationPath.Num() - 1];
		return ActorLocation.Equals(FVector{LastWorldLoc.X, LastWorldLoc.Y, 0.0});
	}

	bool DidComplete(const FVector& ActorLocation, const int Index) const
	{
		const auto CurrentPathLocation = GetCurrentPathLocation(ActorLocation, WorldLocationPath);
		return CurrentPathLocation.has_value() && CurrentPathLocation >= static_cast<double>(Index);
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

	FVector MoveAlongPath(const FVector& ActorLocation, const float DeltaDistance) const
	{
		if (WorldLocationPath.Num() == 0)
			// Then no-op, because there are no path elements.
			return ActorLocation;

		// Compute CurrentPathLocation.
		const auto CurrentPathLocation = GetCurrentPathLocation(ActorLocation, WorldLocationPath);
		if (!CurrentPathLocation.has_value())
		{
			check(false);
			return ActorLocation;
		}
		
		// Compute NewPathLocation.
		const auto NewPathLocation = CurrentPathLocation.value() + DeltaDistance;

		// If we surpassed the length of the path, clamp to the length of the path.
		if (const double PathLen = (WorldLocationPath.Num() - 1) * 100.0; NewPathLocation >= PathLen)
			return WorldLocationPath[WorldLocationPath.Num() - 1];

		// From that single value, convert back to an ActorLocation that is along the path,
		// and return.
		if (NewPathLocation >= 0.0)
		{
			const auto CurrentIndex = FMath::FloorToInt(NewPathLocation * 0.01);
			const auto CurrentNode = WorldLocationPath[CurrentIndex];
			const auto NextNode = WorldLocationPath[CurrentIndex + 1];
			const auto T = FMathHelpers::NegativeFriendlyFmod(NewPathLocation, 100.0) * 0.01;
			return FMathHelpers::Lerp(CurrentNode, NextNode, T);
		}

		// Compute the direction towards the StartNode.
		const auto Dir = (WorldLocationPath[0] - ActorLocation).GetSafeNormal();
		checkf(!Dir.IsNearlyZero(), TEXT("If this assertion is violated you may want to return WorldLocationPath[0] instead."));

		// Then use the direction along with CurrentPathLocation to compute the new ActorLocation.
		// Remember that NewPathLocation is negative.
		return WorldLocationPath[0] + Dir * NewPathLocation;
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

	TArray<FVector> GetWorldLocationPath() const
	{
		return WorldLocationPath;
	}
};
