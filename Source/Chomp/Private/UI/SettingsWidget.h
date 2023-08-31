#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;

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

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
	
	// Revert the Pending* fields to the values returned by GameUserSettings.
	UFUNCTION(BlueprintCallable)
	void RevertPendingState();

	// Update the display fields of UUserWidgets on the screen.
	UFUNCTION(BlueprintCallable)
	void Render();

	// Encapsulate some thorny logic when updating window mode.
	void UpdateWindowMode(const int NewWindowMode);

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

	// Encapsulate some duplicated logic when updating screen resolution.
	void HandleResolutionButtonClicked(std::function<int(int, int)> UpdateResolutionIndex);

	UFUNCTION(BlueprintCallable)
	void HandleResolutionLeftButtonClicked();

	UFUNCTION(BlueprintCallable)
	void HandleResolutionRightButtonClicked();

	UFUNCTION(BlueprintCallable)
	void UpdateEnabledStateOfActionButtons();

	UFUNCTION(BlueprintCallable)
	void HandleApplyClicked();
};
