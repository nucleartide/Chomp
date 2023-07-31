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

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* StartButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StartButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButtonBullet;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitButtonBullet;

	virtual void NativeConstruct() override;

private:
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
};
