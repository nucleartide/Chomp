#pragma once

#include "CoreMinimal.h"
#include "LevelGenerator/LevelLoader.h"
#include "GridLocation.generated.h"

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

    static bool IsInBetween(const FVector& Location, const FGridLocation& A, const FGridLocation& B, const ULevelLoader *LevelInstance);

    // Copy assignment operator.
    FGridLocation& operator=(const FGridLocation& Other) {
        X = Other.X;
        Y = Other.Y;
        return *this;
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
        std::size_t operator()(const FGridLocation &ID) const noexcept
        {
            // NOTE: better to use something like boost hash_combine
            return std::hash<int>()(ID.X ^ ID.Y << 16);
        }
    };
}
