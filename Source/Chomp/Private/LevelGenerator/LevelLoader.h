#pragma once

#include <array>
#include <unordered_set>
#include <vector>
#include "CoreMinimal.h"
#include "ILevelLoader.h"
#include "Math/IntPoint.h"
#include "AStar/Graph.h"
#include "AStar/GridLocation.h"
#include "LevelLoader.generated.h"

/**
 * ULevelLoader loads the contents of a level file (.txt file extension) into memory,
 * then also serves as an instance of the loaded level afterward.
 *
 * TODO: This class has too many responsibilities.
 * If time permits, I'd refactor into ULevel (UDataAsset) and ULoadedLevel (UObject),
 * though that takes lower priority than implementing game mechanics and polishing the UX.
 */
UCLASS(Blueprintable)
class ULevelLoader : public UObject, public IGraph, public ILevelLoader
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
	virtual FVector2D GridToWorld(const FGridLocation& GridPosition) const override;

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
	
	bool Passable(const FGridLocation& TestLocation) const;
	
	bool Passable(const FVector& WorldTestLocation) const;

	virtual bool IsWall(const FGridLocation& Location) const override;

	bool IsGhostHouse(const FGridLocation& Location) const;

	bool IsGateTile(const FGridLocation& Location) const;

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

	bool IsIntersectionTile(const FGridLocation& TileToTest) const;

	FGridLocation GetGateTile() const;

	FGridLocation GetRightOutsideGhostHouseTile() const;

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

	std::optional<FGridLocation> GateTile;

	/**
	 * A set of FGridLocations that describe all the tiles within the ghost house in the level.
	 */
	std::unordered_set<FGridLocation> GhostHouseTiles;
	
	std::optional<FGridLocation> RightOutsideGhostHouseTile;

	/**
	 * The cardinal directions.
	 */
	static std::array<FGridLocation, 4> CardinalDirections;

	bool IsLoaded = false;
};
