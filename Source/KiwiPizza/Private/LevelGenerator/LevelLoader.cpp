#include "Actors/LevelLoader.h"

#include <algorithm>

#include "Kismet/KismetMathLibrary.h"
#include "Algo/Reverse.h"

#include "Utils/Debug.h"

ULevelLoader *ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader> &BlueprintClass)
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
    FString FileContent;
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
            auto Character = StringList[X][Y];
            if (Character == 'W' || Character == 'x')
                Walls.insert(FGridLocation{X, Y});
            else if (Character == '-')
                OnlyGoUpTiles.insert(FGridLocation{X, Y});
            else if (Character == ' ')
            {
                // No-op.
            }
            else
                check(false); // Character is not supported in our level format.
        }
    }
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

FVector2D ULevelLoader::GridToWorld(FGridLocation GridPosition)
{
    FVector2D WorldPosition;
    WorldPosition.X = ((float)GridPosition.X - .5f * GetLevelHeight()) * 100.0f;
    WorldPosition.Y = ((float)GridPosition.Y - .5f * GetLevelWidth()) * 100.0f;
    return WorldPosition;
}

FGridLocation ULevelLoader::WorldToGrid(FVector2D WorldPosition)
{
    // Note: this is the inverse operation of GridToWorld().
    FGridLocation GridPosition;
    GridPosition.X = FMath::FloorToFloat(WorldPosition.X * .01f);
    GridPosition.Y = FMath::FloorToFloat(WorldPosition.Y * .01f);
    GridPosition.X += .5f * GetLevelHeight();
    GridPosition.Y += .5f * GetLevelWidth();
    return GridPosition;
}

bool ULevelLoader::Passable(FGridLocation FromNode, FGridLocation ToNode) const
{
    if (FMath::Abs(FromNode.X - ToNode.X) + FMath::Abs(FromNode.Y - ToNode.Y) != 1)
    {
        // DEBUG_LOG(TEXT("Not passable. From: %s, to: %s"), *FromNode.ToString(), *ToNode.ToString());
        return false;
    }

    if (Walls.find(ToNode) != Walls.end())
        return false;
    else if (OnlyGoUpTiles.find(ToNode) != OnlyGoUpTiles.end())
        // Tile is only passable if the "from" tile is directly below.
        return FromNode.X == (ToNode.X - 1);
    else
        return true;
}

bool ULevelLoader::InBounds(FGridLocation Id) const
{
    return 0 <= Id.X && Id.X < GetLevelHeight() && 0 <= Id.Y && Id.Y < GetLevelWidth();
}

std::array<FGridLocation, 4> ULevelLoader::CARDINAL_DIRECTIONS = {
    FGridLocation{1, 0},  // North
    FGridLocation{-1, 0}, // South
    FGridLocation{0, -1}, // East
    FGridLocation{0, 1}}; // West

std::vector<FGridLocation> ULevelLoader::Neighbors(FGridLocation Id) const
{
    std::vector<FGridLocation> Results;

    for (FGridLocation Direction : CARDINAL_DIRECTIONS)
    {
        FGridLocation Current{Id.X, Id.Y};
        FGridLocation Next{Id.X + Direction.X, Id.Y + Direction.Y};
        if (InBounds(Next) && Passable(Current, Next))
            Results.push_back(Next);
    }

    // Apparently we need this for ordering reasons.
    //
    // See this link for details: https://www.redblobgames.com/pathfinding/a-star/implementation.html#troubleshooting-ugly-path
    if ((Id.X + Id.Y) % 2 == 0)
        std::reverse(Results.begin(), Results.end());

    return Results;
}

double ULevelLoader::Cost(FGridLocation FromNode, FGridLocation ToNode) const
{
    check(FMath::Abs(FromNode.X - ToNode.X) + FMath::Abs(FromNode.Y - ToNode.Y) == 1);
    return 1.0; // Arbitrary non-zero constant.
}
