#pragma once

#include <__functional/function.h>

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;

// TODO: maintain your own pending state independent of GameUserSettings

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

	EWindowMode::Type PendingWindowMode;

	bool PendingVSyncEnabled;

	float PendingFrameRateLimit;

	int PendingGraphicsQuality;

	FIntPoint PendingGraphicsResolution;

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
	UButton* RevertButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ApplyButton;

	UFUNCTION(BlueprintCallable)
	void ResetPendingState();
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void Render();
	void UpdateFullscreenMode(int NewFullscreenMode);

	UFUNCTION(BlueprintCallable)
	void HandleWindowModeLeftButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleWindowModeRightButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleVSyncButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleGraphicsLeftButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleGraphicsRightButtonClicked();
	void HandleResolutionButtonClicked(std::function<int(int, int)> SomeFunction);

	UFUNCTION(BlueprintCallable)
	void HandleResolutionLeftButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleResolutionRightButtonClicked();

	UFUNCTION(BlueprintCallable)
	void UpdateEnabledStateOfActionButtons();

	UFUNCTION(BlueprintCallable)
	void HandleApplyClicked();
};
