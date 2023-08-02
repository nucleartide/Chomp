#pragma once

#include "CoreMinimal.h"
#include <functional>

#include "GridLocation.generated.h"

class ULevelLoader;

USTRUCT(BlueprintType, Blueprintable)
struct FGridLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, SimpleDisplay, meta = (Input))
	int X;

	UPROPERTY(EditAnywhere, SimpleDisplay, meta = (Input))
	int Y;

	FString ToString() const;

	bool IsNonZero() const;

	bool IsZero() const;

	FGridLocation operator+(const FGridLocation& IntendedDir) const;

	static bool IsInBetween(const FVector& Location, const FGridLocation& A, const FGridLocation& B,
	                        const ULevelLoader* LevelInstance);

	// Copy assignment operator.
	FGridLocation& operator=(const FGridLocation& Other)
	{
		X = Other.X;
		Y = Other.Y;
		return *this;
	}

	FVector ToFVector() const
	{
		return FVector{static_cast<double>(X), static_cast<double>(Y), 0.0};
	}

	bool IsOppositeDirection(const FGridLocation& Other) const
	{
		const auto IsXZero = X == 0;
		const auto IsYZero = Y == 0;
		const auto IsXOpposite = !IsXZero && X == -Other.X;
		const auto IsYOpposite = !IsYZero && Y == -Other.Y;
		return IsXOpposite && !IsYOpposite || !IsXOpposite && IsYOpposite;
	}

	bool IsTurningCorner(const FGridLocation& Other) const
	{
		const auto IsSame = X == Other.X && Y == Other.Y;
		return !IsSame && !IsOppositeDirection(Other);
	}

	FGridLocation Modulo(const ULevelLoader* LevelInstance) const;
};

// Implement hash function so we can put FGridLocation into an unordered_set.
template <>
struct std::hash<FGridLocation>
{
	std::size_t operator()(const FGridLocation& ID) const noexcept
	{
		// NOTE: better to use something like boost hash_combine
		return std::hash<int>()(ID.X ^ ID.Y << 16);
	}
};

struct FMaybeGridLocation
{
	bool IsValid{false};
	FGridLocation GridLocation{0, 0};

	static FMaybeGridLocation Invalid()
	{
		return FMaybeGridLocation{false, FGridLocation{0, 0}};
	}

	static FMaybeGridLocation Valid(const FGridLocation& GridLocation)
	{
		return FMaybeGridLocation{true, GridLocation};
	}
};

// Overload some operators.
bool operator==(const FGridLocation& A, const FGridLocation& B);
bool operator!=(const FGridLocation& A, const FGridLocation& B);
bool operator<(const FGridLocation& A, const FGridLocation& B);
