#pragma once

#include <array>
#include <unordered_set>
#include <vector>

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Math/IntPoint.h"

#include "Graph.h"
#include "GridLocation.h"

#include "LevelLoader.generated.h"

/**
 * ULevelLoader loads the contents of a level file (.txt file extension) into memory.
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
	FVector2D GridToWorld(FGridLocation GridPosition);

	/**
	 * Given world coordinates, convert them to grid coordinates.
	 *
	 * Note that the grid origin (0,0) is at the bottom-left of the map.
	 */
	FGridLocation WorldToGrid(FVector2D WorldPosition);

	/**
	 * Check whether a grid position is passable. That is, it's not a wall.
	 */
	bool Passable(FGridLocation GridPosition) const;

	/**
	 * Check whether a grid position is within the map boundaries.
	 */
	bool InBounds(FGridLocation GridPosition) const;

	/**
	 * Get the passable neighbor nodes of a node.
	 */
	std::vector<FGridLocation> Neighbors(FGridLocation GridPosition) const override;

	/**
	 * Return the unitless cost of going from FromNode to ToNode.
	 */
	double Cost(FGridLocation FromNode, FGridLocation ToNode) const override;

	/**
	 * A list of strings that describe the loaded level.
	 *
	 * The first string is at x=0, and the last string is at x=number_of_lines-1.
	 *
	 * Note that x is the vertical dimension in Unreal.
	 */
	TArray<FString> StringList;

#if false
	// TODO: after lunch, fill in implementation of new API above.
public:
	// TODO: can we flip the layout of the map, horizontal <> vertical is confusing.
	static std::array<FGridLocation, 4> DIRS; // this can be private
	int NumberOfLinesInLevel = -1; // this can be private
	int LengthOfLineInLevel = -1; // this can be private
	std::unordered_set<FGridLocation> Walls;// this can be private
	void AddWallTile(int X, int Y); // this can be private
	void ClearWalls(); // this can be private, also it's redundant since I have a Clear()
#endif
};
