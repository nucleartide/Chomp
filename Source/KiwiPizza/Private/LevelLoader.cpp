#include "LevelLoader.h"
#include "Kismet/KismetMathLibrary.h"
#include <algorithm>

std::array<FGridLocation, 4> ULevelLoader::DIRS = {
    FGridLocation{1, 0},  // North
    FGridLocation{-1, 0}, // South
    FGridLocation{0, -1}, // East
    FGridLocation{0, 1}}; // West

ULevelLoader *ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader> &BlueprintClass)
{
    auto DefaultObject = BlueprintClass.GetDefaultObject();
    check(DefaultObject);

    auto Level = Cast<ULevelLoader>(DefaultObject);
    check(Level);

    return Level;
}

int ULevelLoader::GetLevelWidth() const
{
    check(NumberOfLinesInLevel % 2 == 0);
    check(NumberOfLinesInLevel != 0);
    return NumberOfLinesInLevel;
}

int ULevelLoader::GetLevelHeight() const
{
    check(LengthOfLineInLevel % 2 == 0);
    check(LengthOfLineInLevel != 0);
    return LengthOfLineInLevel;
}

FVector2D ULevelLoader::GridToWorld(FGridLocation GridPosition)
{
    FVector2D WorldPosition;
    WorldPosition.X = ((float)GridPosition.X - .5f * GetLevelHeight()) * 100.0f;
    WorldPosition.Y = ((float)GridPosition.Y - .5f * GetLevelWidth()) * 100.0f;
    return WorldPosition;
}

// Inverse operation of GridToWorld().
FGridLocation ULevelLoader::WorldToGrid(FVector2D WorldPosition)
{
    FGridLocation GridPosition;
    GridPosition.X = FMath::FloorToFloat(WorldPosition.X * .01f);
    GridPosition.Y = FMath::FloorToFloat(WorldPosition.Y * .01f);
    GridPosition.X += .5f * GetLevelHeight();
    GridPosition.Y += .5f * GetLevelWidth();
    return GridPosition;
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
    NumberOfLinesInLevel = StringList.Num();

    // Save line length.
    for (int32 x = 0; x < StringList.Num(); x++)
    {
        auto Element = StringList[x];
        LengthOfLineInLevel = Element.Len();
        break;
    }
}

bool ULevelLoader::InBounds(FGridLocation Id) const
{
    return 0 <= Id.X && Id.X < GetLevelHeight() && 0 <= Id.Y && Id.Y < GetLevelWidth();
}

bool ULevelLoader::Passable(FGridLocation Id) const
{
    return Walls.find(Id) == Walls.end();
}

// Get the neighbors of a FGridLocation.
std::vector<FGridLocation> ULevelLoader::Neighbors(FGridLocation Id) const
{
    std::vector<FGridLocation> Results;

    for (FGridLocation Direction : DIRS)
    {
        FGridLocation Next{Id.X + Direction.X, Id.Y + Direction.Y};
        if (InBounds(Next) && Passable(Next))
            Results.push_back(Next);
    }

    // Not 100% sure why we need this, but see this link for explanations: https://www.redblobgames.com/pathfinding/a-star/implementation.html#troubleshooting-ugly-path
    if ((Id.X + Id.Y) % 2 == 0)
    {
        // Reverse the list of neighbors.
        std::reverse(Results.begin(), Results.end());
    }

    return Results;
}

// Add an individual wall tile.
void ULevelLoader::AddWallTile(int X, int Y)
{
    Walls.insert(FGridLocation{X, Y});
}

void ULevelLoader::ClearWalls()
{
    Walls.clear();
}

bool ULevelLoader::IsWall(int X, int Y)
{
    return Walls.find(FGridLocation{X, Y}) != Walls.end();
}

double ULevelLoader::Cost(FGridLocation FromNode, FGridLocation ToNode) const
{
    // NOTE: It is assumed that FromNode and ToNode are neighbors.
    return 1.0; // Arbitrary non-zero constant.
}
