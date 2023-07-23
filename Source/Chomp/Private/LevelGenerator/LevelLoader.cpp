#include "LevelGenerator/LevelLoader.h"

#include <algorithm>

#include "Kismet/KismetMathLibrary.h"
#include "Algo/Reverse.h"

#include "Constants/GameplayTag.h"
#include "Utils/Debug.h"
#include "Engine/World.h"

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
            else if (Character == ' ' || Character == 'o') // ' ' = dot, 'o' = no dot
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

FVector2D ULevelLoader::GridToWorld(FGridLocation GridPosition) const
{
    FVector2D WorldPosition;
    WorldPosition.X = ((float)GridPosition.X - .5f * GetLevelHeight()) * 100.0f;
    WorldPosition.Y = ((float)GridPosition.Y - .5f * GetLevelWidth()) * 100.0f;
    return WorldPosition;
}

FGridLocation ULevelLoader::WorldToGrid(FVector2D WorldPosition) const
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
    FGridLocation GridDirection;

    // Compute X direction.
    if (fmod(WorldPosition.X, 1.0) < 0.5)
        GridDirection.X = -1;
    else
        GridDirection.X = 1;

    // Compute Y direction.
    if (fmod(WorldPosition.Y, 1.0) < 0.5)
        GridDirection.Y = -1;
    else
        GridDirection.Y = 1;

    return GridDirection;
}

bool ULevelLoader::Passable(FGridLocation FromNode, FGridLocation ToNode) const
{
    if (FMath::Abs(FromNode.X - ToNode.X) + FMath::Abs(FromNode.Y - ToNode.Y) != 1)
    {
        // DEBUG_LOG(TEXT("Not passable. From: %s, to: %s"), *FromNode.ToString(), *ToNode.ToString());
        return false;
    }

    // Special case for a 1-unit border around the level, so that ghosts can pathfind to the wrap-around points in the level.
    if (ToNode.X == -1 || ToNode.X == GetLevelHeight() || ToNode.Y == -1 || ToNode.Y == GetLevelWidth())
    {
        return true;
    }

    if (Walls.find(ToNode) != Walls.end())
        return false;
    else if (OnlyGoUpTiles.find(ToNode) != OnlyGoUpTiles.end())
        // Tile is only passable if the "from" tile is directly below.
        return FromNode.X == (ToNode.X - 1);
    else
        return true;
}

FComputeTargetTileResult ULevelLoader::ComputeTargetTile(UWorld *World, FVector Location, FGridLocation Direction, TArray<FName> CollidingTags) const
{
    // Compute the actor's collision sphere.
	auto ActorDiameter = 100.0f - 1.0f; // Needs to be slightly less than 100.0f to avoid overlapping.
    auto ActorRadius =  ActorDiameter * 0.5f;
	auto ActorSphere = FCollisionShape::MakeSphere(ActorRadius);

    // Given the current Position and Direction, compute the target position, but do not set the TargetTile reference just yet.
    FVector StartPos = Location;
    FVector TargetPos = Location;
    TargetPos.X += Direction.X * 100.0f;
    TargetPos.Y += Direction.Y * 100.0f;

    FComputeTargetTileResult Result;

    // Perform an overlap check at the target position.
    TArray<FHitResult> HitResults;
    World->SweepMultiByChannel(HitResults, StartPos, TargetPos, FQuat::Identity, ECC_Visibility, ActorSphere);
    for (auto HitResult : HitResults)
    {
        // If we overlapped with a collider, then we can't travel to target position. Return false.
        auto HitActor = HitResult.GetActor();
        if (GameplayTag::ActorHasOneOf(HitActor, CollidingTags))
        {
            Result.IsValid = false;
            return Result;
        }
    }

    // Otherwise, set the TargetTile (from TargetPos) and return true.
    FVector2D TargetPos2D{TargetPos.X, TargetPos.Y};
    Result.IsValid = true;
    Result.Tile = WorldToGrid(TargetPos2D);
    return Result;
}

bool ULevelLoader::InBounds(FGridLocation Id) const
{
    // Add a 1-unit border around the level because of the wrap-around points in the level.
    return -1 <= Id.X && Id.X <= GetLevelHeight() && -1 <= Id.Y && Id.Y <= GetLevelWidth();
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
