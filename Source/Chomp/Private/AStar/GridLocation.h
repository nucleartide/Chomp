#pragma once

#include "CoreMinimal.h"
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
};

// Overload some operators.
bool operator==(FGridLocation a, FGridLocation b);
bool operator!=(FGridLocation a, FGridLocation b);
bool operator<(FGridLocation a, FGridLocation b);

// Implement hash function so we can put FGridLocation into an unordered_set.
namespace std
{
    template <>
    struct hash<FGridLocation>
    {
        std::size_t operator()(const FGridLocation &id) const noexcept
        {
            // NOTE: better to use something like boost hash_combine
            return std::hash<int>()(id.X ^ (id.Y << 16));
        }
    };
}