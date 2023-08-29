#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HighScoreSubsystem.generated.h"

class ULevelDataAsset;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API UHighScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	int HighScore = 0;

	UPROPERTY(VisibleInstanceOnly)
	bool IsHighScoreNew = false;

public:
	int GetHighScore() const;

	bool GetIsHighScoreNew() const;

	TSubclassOf<UUserWidget> GetHighScoreBonusSymbolWidget() const;
};
