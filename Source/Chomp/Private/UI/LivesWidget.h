#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LivesWidget.generated.h"

class UHorizontalBox;

UCLASS(BlueprintType, Blueprintable)
class ULivesWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UHorizontalBox* LivesContainer;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	class UTextBlock* OneUpText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> LifeWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double HorizontalPadding = 7.5;
};
