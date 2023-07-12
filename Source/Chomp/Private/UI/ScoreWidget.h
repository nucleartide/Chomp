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

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock *ScoreTextBlock;

	UFUNCTION()
	void HandleScoreUpdated(int Score);
};
