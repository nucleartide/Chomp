#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "ScoreWidget.generated.h"

UCLASS(Blueprintable)
class UScoreWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock *ScoreTextBlock;
	
	UFUNCTION(BlueprintCallable)
	void UpdateScoreText(int Score);
};
