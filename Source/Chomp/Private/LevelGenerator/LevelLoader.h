#pragma once

#include <array>
#include <unordered_set>
#include <vector>
#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "AStar/Graph.h"
#include "AStar/GridLocation.h"
#include "LevelLoader.generated.h"

USTRUCT()
struct FComputeTargetTileResult
{
	GENERATED_BODY()

	bool IsValid;
	FGridLocation Tile;

	static FComputeTargetTileResult Invalid()
	{
		constexpr FGridLocation Location{0, 0};
		constexpr FComputeTargetTileResult Result{false, Location};
		return Result;
	}
};

/**
 * ULevelLoader loads the contents of a level file (.txt file extension) into memory,
 * then also serves as an instance of the loaded level afterward.
 *
 * TODO: This could probably be split into 2 classes: ULevelLoader, and ULevel. Not that important though.
 */
UCLASS(Blueprintable)
class ULevelLoader : public UObject, public IGraph
{
	GENERATED_BODY()

public:
	/**
	 * The path of the file to load from our Content/ directory.
	 *
	 * Example: "Levels/level2.txt"
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Level Filename Customization")
	FString LevelFilename = TEXT("Levels/level2.txt");

	/**
	 * Grab a reference to a Blueprint asset's default instance of ULevelLoader.
	 */
	static ULevelLoader *GetInstance(const TSubclassOf<ULevelLoader> &BlueprintClass);

	/**
	 * Load the contents of a level file into this instance of ULevelLoader.
	 */
	void LoadLevel();

	/**
	 * Clear any loaded level content within this instance of ULevelLoader.
	 */
	void Clear();

	/**
	 * Get the width (horizontal dimension) of the level.
	 */
	int GetLevelWidth() const;

	/**
	 * Get the height (vertical dimension) of the level.
	 */
	int GetLevelHeight() const;

	/**
	 * Given grid coordinates, convert them to world coordinates.
	 *
	 * Note that the grid origin (0,0) is at the bottom-left of the map.
	 */
	FVector2D GridToWorld(const FGridLocation& GridPosition) const;

	/**
	 * Given world coordinates, convert them to grid coordinates.
	 *
	 * Note that the grid origin (0,0) is at the bottom-left of the map.
	 */
	FGridLocation WorldToGrid(FVector2D WorldPosition) const;

	/**
	 * Given world coordinates, compute the direction required to snap the world coordinates to the closest grid position.
	 *
	 * Note that the grid origin (0,0) is at the bottom-left of the map.
	 */
	static FGridLocation SnapToGridDirection(FVector2D WorldPosition);

	/**
	 * Check whether ToLocation is passable, given that we're coming from FromLocation.
	 */
	bool Passable(const FGridLocation& FromLocation, const FGridLocation& ToLocation) const;

	bool IsWall(const FGridLocation& Location) const;

	FComputeTargetTileResult ComputeTargetTile(UWorld *World, FVector Location, FGridLocation Direction, const TArray<FName> &TagsToCollideWith, FString DebugLabel) const;

	/**
	 * Check whether a grid position is within the map boundaries.
	 */
	bool InBounds(const FGridLocation& GridPosition) const;

	/**
	 * Get the passable neighbor nodes of a node.
	 */
	virtual std::vector<FGridLocation> Neighbors(FGridLocation GridPosition) const override;

	/**
	 * Return the unitless cost of going from FromNode to ToNode.
	 */
	virtual double Cost(FGridLocation FromNode, FGridLocation ToNode) const override;

	/**
	 * A list of strings that describe the loaded level.
	 *
	 * The first string is at x=0, and the last string is at x=number_of_lines-1.
	 *
	 * Note that x is the vertical dimension in Unreal.
	 */
	TArray<FString> StringList;

private:
	/**
	 * The number of rows in the level.
	 */
	int NumberOfRows = 0;

	/**
	 * The number of columns in the level.
	 */
	int NumberOfColumns = 0;

	/**
	 * A set of FGridLocations that describe all the wall tiles in the level.
	 */
	std::unordered_set<FGridLocation> Walls;

	/**
	 * A set of FGridLocations that describe all the "OnlyGoUp" tiles in the level.
	 */
	std::unordered_set<FGridLocation> OnlyGoUpTiles;

	/**
	 * The cardinal directions.
	 */
	static std::array<FGridLocation, 4> CardinalDirections;
};
