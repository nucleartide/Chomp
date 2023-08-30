#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ChompSaveGame.generated.h"

class ULevelDataAsset;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API UChompSaveGame : public USaveGame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "High Score")
	int HighScore = 0;
	
	UPROPERTY(EditAnywhere, Category = "High Score")
	ULevelDataAsset* HighScoreLevel;

	UPROPERTY(VisibleInstanceOnly, Category = "High Score")
	bool IsDirty = false;

public:
	static UChompSaveGame* Load(
		const FString SaveSlotName = TEXT("ChompSaveSlot"),
		const int UserIndex = 0
	);

	void Save(
		const FString SaveSlotName = TEXT("ChompSaveSlot"),
		const int UserIndex = 0
	);

	int GetHighScore() const;

	void SetHighScore(int NewHighScore);

	bool IsHighScoreNew() const;

	ULevelDataAsset* GetHighScoreLevel() const;

	void SetHighScoreLevel(ULevelDataAsset* NewHighScoreLevel);
};
