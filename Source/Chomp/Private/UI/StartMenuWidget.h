#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "StartMenuWidget.generated.h"

class UButton;

UCLASS(Blueprintable, BlueprintType)
class CHOMP_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StartButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitButtonBullet;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleStartGame();

	UFUNCTION()
	void HandleOptionsButtonClicked();

	UFUNCTION()
	void HandleQuitGame();

	UFUNCTION()
	void HandleStartHover();
};
