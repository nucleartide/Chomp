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
	if (IsLoaded)
		return;
	else
		IsLoaded = true;

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
			else if (Character == 'S')
			{
				WrapAroundTiles.insert(FGridLocation{X, Y});
			}
			else if (Character == '-')
			{
				checkf(!GateTile.has_value(),
				       TEXT("GateTile is being set multiple times. You have too many '-' chars in your level file!"));
				GateTile = FGridLocation{X, Y};
			}
			else if (Character == 'G') // 'G' = inside ghost house
			{
				GhostHouseTiles.insert(FGridLocation{X, Y});
			}
			else if (Character == 'g') // 'g' = right outside ghost house
			{
				checkf(!RightOutsideGhostHouseTile.has_value(),
				       TEXT(
					       "RightOutsideGhostHouseTile is being set multiple times. You have too many 'g' chars in your level file!"
				       ));
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
	check(GateTile.has_value());
	check(WrapAroundTiles.size() > 0);
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
	if (!AreWithinOneUnit(FromLocation, ToLocation))
		return false;

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

bool ULevelLoader::IsWrapAround(const FGridLocation& Location) const
{
	return WrapAroundTiles.find(Location) != WrapAroundTiles.end();
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

std::vector<FGridLocation> ULevelLoader::Neighbors(const FGridLocation GridPosition) const
{
	// Construct set of adjacent nodes, starting with cardinal directions.
	std::vector<FGridLocation> AdjacentNodes;
	for (const auto& Dir : CardinalDirections)
	{
		const auto Adj = GridPosition + Dir;
		AdjacentNodes.push_back(Adj);
	}

	// Then, if the GridPosition is a wrap-around node, also add the corresponding wrap-around node.
	if (IsWrapAround(GridPosition))
	{
		if (GridPosition.X == 0 || GridPosition.X == GetLevelHeight() - 1)
		{
			check(GridPosition.Y != 0 && GridPosition.Y != GetLevelWidth() - 1);
			const auto Inverse = FGridLocation{GetLevelHeight() - 1 - GridPosition.X, GridPosition.Y};
			AdjacentNodes.push_back(Inverse);
		}
		else if (GridPosition.Y == 0 || GridPosition.Y == GetLevelWidth() - 1)
		{
			check(GridPosition.X != 0 && GridPosition.X != GetLevelHeight() - 1);
			const auto Inverse = FGridLocation{GridPosition.X, GetLevelWidth() - 1 - GridPosition.Y};
			AdjacentNodes.push_back(Inverse);
		}
	}

	std::vector<FGridLocation> Results;
	for (const auto& Adj : AdjacentNodes)
	{
		if (Passable(GridPosition, Adj))
			Results.push_back(Adj);
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
	check(AreWithinOneUnit(FromNode, ToNode));
	if (AreOnOppositeEnds(FromNode, ToNode))
	{
		return 5.0; // Greater cost when wrapping around.
	}

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
	return GetTile(GateTile, this);
}

FGridLocation ULevelLoader::GetRightOutsideGhostHouseTile() const
{
	return GetTile(RightOutsideGhostHouseTile, this);
}

FGridLocation ULevelLoader::GetTile(std::optional<FGridLocation> MaybeTile, const ULevelLoader* LevelInstance)
{
	check(MaybeTile.has_value());
	check(LevelInstance->InBounds(MaybeTile.value()));
	return MaybeTile.value();
}

bool ULevelLoader::AreWithinOneUnit(const FGridLocation& A, const FGridLocation& B) const
{
	const auto XDist = FMath::Abs(A.X - B.X);
	const auto MinXDist = FMath::Min(XDist, GetLevelHeight() - XDist);
	const auto YDist = FMath::Abs(A.Y - B.Y);
	const auto MinYDist = FMath::Min(YDist, GetLevelWidth() - YDist);
	return MinXDist + MinYDist == 1;
}

bool ULevelLoader::AreOnOppositeEnds(const FGridLocation& A, const FGridLocation& B) const
{
	const auto AreOnOppositeEndsOfX = FMath::Abs(A.X - B.X) == GetLevelHeight() - 1 && A.Y == B.Y;
	const auto AreOnOppositeEndsOfY = FMath::Abs(A.Y - B.Y) == GetLevelWidth() - 1 && A.X == B.X;
	return AreOnOppositeEndsOfX || AreOnOppositeEndsOfY;
}
