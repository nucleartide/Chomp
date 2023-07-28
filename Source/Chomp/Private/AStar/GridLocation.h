#pragma once

#include "CoreMinimal.h"
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
};

struct FMaybeGridLocation
{
	bool IsValid;
	FGridLocation GridLocation;

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

// Implement hash function so we can put FGridLocation into an unordered_set.
namespace std
{
	template <>
	struct hash<FGridLocation>
	{
		std::size_t operator()(const FGridLocation& ID) const noexcept
		{
			// NOTE: better to use something like boost hash_combine
			return std::hash<int>()(ID.X ^ ID.Y << 16);
		}
	};
}
