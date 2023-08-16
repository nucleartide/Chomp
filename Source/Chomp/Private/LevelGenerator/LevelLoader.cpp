#include "LevelGenerator/LevelLoader.h"
#include <algorithm>
#include "Algo/Reverse.h"
#include "Engine/World.h"
#include "AStar/GridLocation.h"

ULevelLoader* ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader>& BlueprintClass)
{
	auto DefaultObject = BlueprintClass.GetDefaultObject();
	check(DefaultObject);

	auto Level = Cast<ULevelLoader>(DefaultObject);
	check(Level);

	return Level;
}

void ULevelLoader::LoadLevel()
{
	// Load file into variable.
	TArray<uint8> FileData;
	FString FilePath = FPaths::ProjectContentDir() + LevelFilename; // Example: "Levels/level2.txt"
	auto Ok = FFileHelper::LoadFileToArray(FileData, *FilePath);
	check(Ok);

	// The FileContent FString now contains the text content of the loaded file.
	FString FileContent{""};
	FFileHelper::BufferToString(FileContent, FileData.GetData(), FileData.Num());

	// Split FileContent into a list of lines.
	FileContent.ParseIntoArray(StringList, TEXT("\n"), true);

	// Save the number of lines.
	NumberOfRows = StringList.Num();
	check(NumberOfRows > 0);

	// Save line length.
	for (int32 x = 0; x < StringList.Num(); x++)
	{
		auto Element = StringList[x];
		NumberOfColumns = Element.Len();
		break;
	}

	// Reverse the StringList so that we can iterate from X=0 to X=NumberOfRows-1,
	// bottom row to top row.
	Algo::Reverse(StringList);

	// Load up our data structures that track blocking tiles.
	for (auto X = 0; X < NumberOfRows; X++)
	{
		for (auto Y = 0; Y < NumberOfColumns; Y++)
		{
			if (auto Character = StringList[X][Y]; Character == 'W' || Character == 'x')
			{
				Walls.insert(FGridLocation{X, Y});
			}
			else if (Character == '-')
			{
				checkf(!GateTile.has_value(), TEXT("GateTile is being set multiple times. You have too many '-' chars in your level file!"));
				GateTile = FGridLocation{X, Y};
			}
			else if (Character == 'G') // 'G' = inside ghost house
			{
				GhostHouseTiles.insert(FGridLocation{X, Y});
			}
			else if (Character == 'g') // 'g' = right outside ghost house
			{
				checkf(!RightOutsideGhostHouseTile.has_value(), TEXT("RightOutsideGhostHouseTile is being set multiple times. You have too many 'g' chars in your level file!"));
				RightOutsideGhostHouseTile = FGridLocation{X, Y};
			}
			else if (Character == ' ' || Character == 'o' || Character == 'O') // ' ' = dot, 'o' = no dot
			{
				// No-op.
			}
			else
			{
				checkf(false, TEXT("Character is not supported in our level format."));
			}
		}
	}

	// Post-conditions.
	check(RightOutsideGhostHouseTile.has_value());
}

void ULevelLoader::Clear()
{
	StringList.Empty();
	NumberOfRows = 0;
	NumberOfColumns = 0;
	Walls.clear();
}

int ULevelLoader::GetLevelWidth() const
{
	check(NumberOfColumns % 2 == 0);
	check(NumberOfColumns != 0);
	return NumberOfColumns;
}

int ULevelLoader::GetLevelHeight() const
{
	check(NumberOfRows % 2 == 0);
	check(NumberOfRows != 0);
	return NumberOfRows;
}

FVector2D ULevelLoader::GridToWorld(const FGridLocation& GridPosition) const
{
	FVector2D WorldPosition;
	WorldPosition.X = (static_cast<float>(GridPosition.X) - .5f * GetLevelHeight()) * 100.0f;
	WorldPosition.Y = (static_cast<float>(GridPosition.Y) - .5f * GetLevelWidth()) * 100.0f;
	return WorldPosition;
}

FGridLocation ULevelLoader::WorldToGrid(const FVector2D WorldPosition) const
{
	// Note: this is the inverse operation of GridToWorld().
	FGridLocation GridPosition;
	GridPosition.X = FMath::RoundToFloat(WorldPosition.X * .01f);
	GridPosition.Y = FMath::RoundToFloat(WorldPosition.Y * .01f);
	GridPosition.X += .5f * GetLevelHeight();
	GridPosition.Y += .5f * GetLevelWidth();
	return GridPosition;
}

FGridLocation ULevelLoader::SnapToGridDirection(const FVector2D WorldPosition)
{
	FGridLocation GridDirection{0, 0};
	const auto FractionX = fmod(WorldPosition.X, 1.0);
	const auto FractionY = fmod(WorldPosition.Y, 1.0);

	if (FractionX == 0.0 && FractionY == 0.0)
		return GridDirection;

	if (FractionX != 0.0)
		GridDirection.X = FractionX < 0.5 ? -1 : 1;

	if (FractionY != 0.0)
		GridDirection.Y = FractionY < 0.5 ? -1 : 1;

	check(GridDirection.X != GridDirection.Y);
	return GridDirection;
}

bool ULevelLoader::Passable(const FGridLocation& FromLocation, const FGridLocation& ToLocation) const
{
	if (FMath::Abs(FromLocation.X - ToLocation.X) + FMath::Abs(FromLocation.Y - ToLocation.Y) != 1)
	{
		return false;
	}

#if false
	// Don't include this, then ghosts can't return to ghost house.
	// If ToLocation is an OnlyGoUpTile,
	if (GateTiles.find(ToLocation) != GateTiles.end())
	{
		// Then ToLocation is only passable if FromLocation is directly below.
		return ToLocation.Y == FromLocation.Y && ToLocation.X == FromLocation.X + 1;
	}
#endif

	return Passable(ToLocation);
}

bool ULevelLoader::Passable(const FGridLocation& TestLocation) const
{
	// First thing to check.
	if (!InBounds(TestLocation))
	{
		return false;
	}

	// If TestLocation is a Wall,
	if (Walls.find(TestLocation) != Walls.end())
	{
		// Then TestLocation is not passable.
		return false;
	}

	return true;
}

bool ULevelLoader::Passable(const FVector& WorldTestLocation) const
{
	const auto GridLocation = WorldToGrid(FVector2D(WorldTestLocation));
	return Passable(GridLocation);
}

bool ULevelLoader::IsWall(const FGridLocation& Location) const
{
	return Walls.find(Location) != Walls.end();
}

bool ULevelLoader::IsGhostHouse(const FGridLocation& Location) const
{
	return GhostHouseTiles.find(Location) != GhostHouseTiles.end();
}

bool ULevelLoader::IsGateTile(const FGridLocation& Location) const
{
	check(GateTile.has_value());
	return GateTile.value() == Location;
}

bool ULevelLoader::InBounds(const FGridLocation& GridPosition) const
{
	return 0 <= GridPosition.X
		&& GridPosition.X < GetLevelHeight()
		&& 0 <= GridPosition.Y
		&& GridPosition.Y < GetLevelWidth();
}

std::array<FGridLocation, 4> ULevelLoader::CardinalDirections = {
	FGridLocation{1, 0}, // North
	FGridLocation{-1, 0}, // South
	FGridLocation{0, -1}, // East
	FGridLocation{0, 1} // West
};

// TODO: Add support for wraparound positions in this Neighbors function.
// You can probably pass in a set of special wraparound positions.
// Currently ghosts will never traverse to the wraparound positions.
std::vector<FGridLocation> ULevelLoader::Neighbors(const FGridLocation GridPosition) const
{
	std::vector<FGridLocation> Results;

	for (const auto [X, Y] : CardinalDirections)
	{
		FGridLocation Current{GridPosition.X, GridPosition.Y};
		if (FGridLocation Next{GridPosition.X + X, GridPosition.Y + Y}; Passable(Current, Next))
			Results.push_back(Next);
	}

	// Apparently we need this for ordering reasons.
	//
	// See this link for details: https://www.redblobgames.com/pathfinding/a-star/implementation.html#troubleshooting-ugly-path
	if ((GridPosition.X + GridPosition.Y) % 2 == 0)
		std::reverse(Results.begin(), Results.end());

	return Results;
}

double ULevelLoader::Cost(FGridLocation FromNode, FGridLocation ToNode) const
{
	check(FMath::Abs(FromNode.X - ToNode.X) + FMath::Abs(FromNode.Y - ToNode.Y) == 1);
	return 1.0; // Arbitrary non-zero constant.
}

bool ULevelLoader::IsIntersectionTile(const FGridLocation& TileToTest) const
{
	const auto AdjacentTiles = Neighbors(TileToTest);
	int VerticallyAdjacentTiles = 0;
	int HorizontallyAdjacentTiles = 0;

	for (const auto& AdjacentTile : AdjacentTiles)
	{
		if (const auto [X, Y] = AdjacentTile - TileToTest; FMath::Abs(X) == 1)
			VerticallyAdjacentTiles++;
		else if (FMath::Abs(Y) == 1)
			HorizontallyAdjacentTiles++;
	}

	return VerticallyAdjacentTiles >= 1 && HorizontallyAdjacentTiles >= 1;
}

FGridLocation ULevelLoader::GetGateTile() const
{
	check(GateTile.has_value());
	return GateTile.value();
}

FGridLocation ULevelLoader::GetRightOutsideGhostHouseTile() const
{
	check(RightOutsideGhostHouseTile.has_value());
	check(InBounds(RightOutsideGhostHouseTile.value()));
	return RightOutsideGhostHouseTile.value();
}
