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

	UPROPERTY(Transient)
	bool IsDirty;

public:
	static UChompSaveGame* Load(
		const FString SaveSlotName = TEXT("ChompSaveSlot"),
		const int UserIndex = 0
	);

	void Save(
		const FString SaveSlotName = TEXT("ChompSaveSlot"),
		const int UserIndex = 0
	);

	UFUNCTION(BlueprintCallable)
	int GetHighScore() const;

	UFUNCTION(BlueprintCallable)
	void SetHighScore(const int NewHighScore, ULevelDataAsset* NewHighScoreLevel);

	UFUNCTION(BlueprintCallable)
	bool IsHighScoreNew() const;

	UFUNCTION(BlueprintCallable)
	TSubclassOf<UUserWidget> GetHighScoreLevelWidget() const;
};
