#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* WindowModeLeftButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* WindowModeRightButton;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* WindowModeSelection;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* VSyncLeftButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* VSyncRightButton;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* VSyncSelection;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* GraphicsLeftButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* GraphicsRightButton;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* GraphicsSelection;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ResolutionLeftButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ResolutionRightButton;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* ResolutionSelection;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ApplyButton;

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void Render() const;
};
