#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LevelLoader.generated.h"

UCLASS(Blueprintable)
class ULevelLoader : public UObject
{
	GENERATED_BODY()

public:
	int GetLevelWidth();
	int GetLevelHeight();
	void LoadLevel();

private:
	int NumberOfLinesInLevel = -1;
	int LengthOfLineInLevel = -1;

	UPROPERTY(EditDefaultsOnly, Category = "Level Filename Customization")
	FString LevelFilename = TEXT("Levels/level2.txt");

public:
	TArray<FString> StringList;
	// static int GetLevelWidth(TSubclassOf<ULevelLoader> LevelLoader);
	// static int GetLevelHeight(TSubclassOf<ULevelLoader> LevelLoader);
};
