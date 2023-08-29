#pragma once

#include "CoreMinimal.h"
#include "LevelSymbolWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "StartMenuWidget.generated.h"

class UButton;

UCLASS(Blueprintable, BlueprintType)
class CHOMP_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	bool IsHighScoreNew = true;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULevelSymbolWidget> BonusSymbolWidget;

	UFUNCTION()
	void HandleStartHover();

	UFUNCTION()
	void HandleStartUnhover();

	UFUNCTION()
	void HandleOptionsHover();

	UFUNCTION()
	void HandleOptionsUnhover();

	UFUNCTION()
	void HandleQuitHover();

	UFUNCTION()
	void HandleQuitUnhover();

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* HighScoreValue;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UTextBlock* HighScoreNewIndicator;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UHorizontalBox* HighScoreLevelSymbolBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StartButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitButtonBullet;

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	// Note: you should call this function in your Blueprint.
	UFUNCTION(BlueprintCallable)
	void Render(UWorld* WorldInstance) const;
};
