#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Math/IntPoint.h"
#include <array>
#include <unordered_set>
#include <vector>
#include "Graph.h"
#include "GridLocation.h"
#include "LevelLoader.generated.h"

UCLASS(Blueprintable)
class ULevelLoader : public UObject, public IGraph
{
	GENERATED_BODY()

public:
	int GetLevelWidth() const;
	int GetLevelHeight() const;
	void LoadLevel();
	FVector2D GridToWorld(GridLocation GridPosition);
	GridLocation WorldToGrid(FVector2D WorldPosition);

	// This method returns a reference to the Blueprint asset's single instance.
	static ULevelLoader *GetInstance(const TSubclassOf<ULevelLoader>& BlueprintClass);

private:
	int NumberOfLinesInLevel = -1;
	int LengthOfLineInLevel = -1;

	UPROPERTY(EditDefaultsOnly, Category = "Level Filename Customization")
	FString LevelFilename = TEXT("Levels/level2.txt");

public:
	TArray<FString> StringList;
	static std::array<GridLocation, 4> DIRS;
	bool InBounds(GridLocation Id) const;
	bool Passable(GridLocation Id) const;
	std::vector<GridLocation> Neighbors(GridLocation Id) const override;
    double Cost(GridLocation FromNode, GridLocation ToNode) const override;

	// TODO: Gotta initialize these walls.
	std::unordered_set<GridLocation> Walls;

	void AddWallTile(int X, int Y);
	void ClearWalls();
	bool IsWall(int X, int Y);
};
