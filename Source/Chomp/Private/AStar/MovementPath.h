#pragma once

#include <tuple>

#include "GenericPlatform/GenericPlatformMath.h"
#include "GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Utils/Debug.h"
#include "Utils/MathHelpers.h"

#include "MovementPath.generated.h"

USTRUCT(BlueprintType)
struct FMovementPath
{
	GENERATED_BODY()

	friend bool operator==(const FMovementPath& Lhs, const FMovementPath& RHS)
	{
		return Lhs.GridLocationPath == RHS.GridLocationPath;
	}

	friend bool operator!=(const FMovementPath& Lhs, const FMovementPath& Rhs)
	{
		return !(Lhs == Rhs);
	}

private:
	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	UPROPERTY(VisibleAnywhere)
	TArray<FGridLocation> GridLocationPath;

	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> WorldLocationPath;

	// Note: if you are adding more properties, you should also update the overloaded copy assignment operator.
	ILevelLoader* LevelInstance;

	std::optional<double> GetCurrentPathLocation(const FVector& ActorLocation) const
	{
		// Establish where the actor is along the path as a single value.
		auto CurrentPathLocation = 0.0;

		// Traverse the path to see where the actor is currently on the path.
		for (auto i = 0; i < WorldLocationPath.Num() - 1; i++)
		{
			auto CurrentNode = WorldLocationPath[i];
			auto NextNode = WorldLocationPath[i + 1];

			if (const auto Result =
					FGridLocation::IsInBetween(ActorLocation, CurrentNode, NextNode, LevelInstance);
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
		ILevelLoader* LevelInstance) : LevelInstance(LevelInstance)
	{
		// Initialize state.
		GridLocationPath = NewLocationPath;
		for (auto Location : NewLocationPath)
		{
			const auto WorldLocation = LevelInstance->GridToWorld(Location);
			WorldLocationPath.Add(FVector{WorldLocation.X, WorldLocation.Y, 0.0f});
		}

		// Sanity check.
		check(GetCurrentPathLocation(ActorLocation).has_value());
		for (auto Location : NewLocationPath)
			check(!LevelInstance->IsWall(Location));
	}

	void Reset()
	{
		GridLocationPath.Empty();
		WorldLocationPath.Empty();
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
		const auto CurrentPathLocation = GetCurrentPathLocation(ActorLocation);
		check(CurrentPathLocation.has_value());
		check(
			-50.0 <= CurrentPathLocation.value() &&
			CurrentPathLocation.value() <= (GridLocationPath.Num() - 1) * 100.0
		);
		return CurrentPathLocation >= (static_cast<double>(Index) * 100.0);
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

	static FVector WrapFriendlyLerp(const FVector& A, const FVector& B, double T, ILevelLoader* LevelInstance)
	{
		// Pre-conditions.
		check(0.0 <= T && T <= 1.0);
		checkf(
			(
				FMath::Abs(A.X - B.X) <= 100.0 ||
				FMath::Abs(A.X - B.X) >= (LevelInstance->GetLevelHeight() - 1) * 100.0
			) &&
			(
				FMath::Abs(A.Y - B.Y) <= 100.0 ||
				FMath::Abs(A.Y - B.Y) >= (LevelInstance->GetLevelWidth() - 1) * 100.0
			),
			TEXT("A and B are within one unit of each other.")
		);

		const auto XDiff = FMath::Abs(A.X - B.X);
		const auto YDiff = FMath::Abs(A.Y - B.Y);

		auto NewX = 0.0;
		auto NewY = 0.0;

		// Special case.
		if (XDiff >= (LevelInstance->GetLevelHeight() - 1) * 100.0)
		{
			const auto Ax = A.X < 0.0 ? A.X + LevelInstance->GetLevelHeight() * 100.0 : A.X;
			const auto Bx = B.X < 0.0 ? B.X + LevelInstance->GetLevelHeight() * 100.0 : B.X;
			const auto IntermediateResult = FMath::Lerp(Ax, Bx, T);
			NewX = IntermediateResult >= LevelInstance->GetLevelHeight() * 50.0 - 50.0 // half
				       ? IntermediateResult - LevelInstance->GetLevelHeight() * 100.0
				       : IntermediateResult;
		}
		// Normal case.
		else
		{
			NewX = FMath::Lerp(A.X, B.X, T);
		}
		
		// Special case.
		if (YDiff >= (LevelInstance->GetLevelWidth() - 1) * 100.0)
		{
			const auto Ay = A.Y < 0.0 ? A.Y + LevelInstance->GetLevelWidth() * 100.0 : A.Y;
			const auto By = B.Y < 0.0 ? B.Y + LevelInstance->GetLevelWidth() * 100.0 : B.Y;
			const auto IntermediateResult = FMath::Lerp(Ay, By, T);
			NewY = IntermediateResult >= LevelInstance->GetLevelWidth() * 50.0 - 50.0 // half
				       ? IntermediateResult - LevelInstance->GetLevelWidth() * 100.0
				       : IntermediateResult;
		}
		// Normal case.
		else
		{
			NewY = FMath::Lerp(A.Y, B.Y, T);
		}

		return FVector{NewX, NewY, 0.0};
	}

	FVector MoveAlongPath(const FVector& ActorLocation, const float DeltaDistance) const
	{
		if (WorldLocationPath.Num() == 0)
			// Then no-op, because there are no path elements.
			return ActorLocation;

		// Compute CurrentPathLocation.
		const auto CurrentPathLocation = GetCurrentPathLocation(ActorLocation);
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
			return WrapFriendlyLerp(CurrentNode, NextNode, T, LevelInstance);
		}

		// Compute the direction towards the StartNode.
		const auto Dir = (WorldLocationPath[0] - ActorLocation).GetSafeNormal();
		check((WorldLocationPath[0] - ActorLocation).Length() <= 100.0);

		// Then use the direction along with NewPathLocation to compute the new ActorLocation.
		// Remember that NewPathLocation is negative here.
		// Also note that if Dir is a zero vector, this result reduces to WorldLocationPath[0].
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

	TArray<FGridLocation> GetGridLocationPath() const
	{
		return GridLocationPath;
	}
};
