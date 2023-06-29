// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelLoader.h"

ULevelLoader *ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader>& BlueprintClass)
{
	auto DefaultObject = BlueprintClass.GetDefaultObject();
	check(DefaultObject);

	auto Level = Cast<ULevelLoader>(DefaultObject);
	check(Level);

    return Level;
}

int ULevelLoader::GetLevelWidth()
{
    return NumberOfLinesInLevel;
}

int ULevelLoader::GetLevelHeight()
{
    return LengthOfLineInLevel;
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
