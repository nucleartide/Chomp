#include "LevelLoader.h"
#include "Kismet/KismetMathLibrary.h"

ULevelLoader *ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader> &BlueprintClass)
{
    auto DefaultObject = BlueprintClass.GetDefaultObject();
    check(DefaultObject);

    auto Level = Cast<ULevelLoader>(DefaultObject);
    check(Level);

    return Level;
}

int ULevelLoader::GetLevelWidth()
{
    check(NumberOfLinesInLevel % 2 == 0);
    check(NumberOfLinesInLevel != 0);
    return NumberOfLinesInLevel;
}

int ULevelLoader::GetLevelHeight()
{
    check(LengthOfLineInLevel % 2 == 0);
    check(LengthOfLineInLevel != 0);
    return LengthOfLineInLevel;
}

FVector2D ULevelLoader::GridToWorld(FVector2D GridPosition)
{
    FVector2D WorldPosition;
    WorldPosition.X = (FMath::FloorToFloat(GridPosition.X) - .5f * GetLevelHeight()) * 100.0f;
    WorldPosition.Y = (FMath::FloorToFloat(GridPosition.Y) - .5f * GetLevelWidth()) * 100.0f;
    return WorldPosition;
}

// Inverse operation of GridToWorld().
FVector2D ULevelLoader::WorldToGrid(FVector2D WorldPosition)
{
    FVector2D GridPosition;
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
