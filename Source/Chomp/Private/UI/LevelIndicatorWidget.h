#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "LevelIndicatorWidget.generated.h"

class ULevelSymbolWidget;
class ULevelSequenceDataAsset;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API ULevelIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* LevelIndicatorContainer;

	UPROPERTY(EditDefaultsOnly)
	ULevelSequenceDataAsset* LevelSequence;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LevelSymbolWidget;

public:
	void RenderLastThreeLevelSymbols(UWorld* WorldInstance) const;
};
