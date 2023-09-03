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

protected:
	UPROPERTY(EditAnywhere, Category = "LevelIndicatorWidget | Props", BlueprintReadOnly)
	ULevelSequenceDataAsset* LevelSequence;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UHorizontalBox* LevelIndicatorContainer;

public:
	void RenderLastThreeLevelSymbols(UWorld* WorldInstance) const;
};
