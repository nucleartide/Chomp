#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Math/IntPoint.h"
#include <array>
#include <unordered_set>
#include <vector>
#include "LevelLoader.generated.h"

using GridLocation = FIntPoint;

namespace std
{
	/* implement hash function so we can put GridLocation into an unordered_set */
	template <>
	struct hash<GridLocation>
	{
		std::size_t operator()(const GridLocation &id) const noexcept
		{
			// NOTE: better to use something like boost hash_combine
			return std::hash<int>()(id.X ^ (id.Y << 16));
		}
	};
}

UCLASS(Blueprintable)
class ULevelLoader : public UObject
{
	GENERATED_BODY()

public:
	int GetLevelWidth() const;
	int GetLevelHeight() const;
	void LoadLevel();
	FVector2D GridToWorld(FIntPoint GridPosition);
	FIntPoint WorldToGrid(FVector2D WorldPosition);

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
	std::vector<GridLocation> Neighbors(GridLocation Id) const;

	// TODO: Gotta initialize these walls.
	std::unordered_set<GridLocation> Walls;

	void AddWallTile(int X, int Y);
	void ClearWalls();
	bool IsWall(int X, int Y);
};
