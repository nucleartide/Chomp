#include "SettingsWidget.h"
#include <functional>
#include "DynamicRHI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utils/MathHelpers.h"

void USettingsWidget::RevertPendingState()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	// Initialize component state.
	PendingWindowMode = GameUserSettings->GetFullscreenMode();
	PendingVSyncEnabled = GameUserSettings->IsVSyncEnabled();
	PendingGraphicsQuality = GameUserSettings->GetOverallScalabilityLevel();
	PendingGraphicsResolution = GameUserSettings->GetScreenResolution();
	PendingFrameRateLimit = GameUserSettings->GetFrameRateLimit();

	Render();
}

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WindowModeLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeButtonClicked);
	WindowModeRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeButtonClicked);

	VSyncLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);
	VSyncRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);

	GraphicsLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsLeftButtonClicked);
	GraphicsRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsRightButtonClicked);

	ResolutionLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleResolutionLeftButtonClicked);
	ResolutionRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleResolutionRightButtonClicked);

	RevertButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::RevertPendingState);
	ApplyButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleApplyClicked);

	RevertPendingState();
	Render();
}

void USettingsWidget::NativeDestruct()
{
	Super::NativeDestruct();

	WindowModeLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeButtonClicked);
	WindowModeRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeButtonClicked);

	VSyncLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);
	VSyncRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);

	GraphicsLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleGraphicsLeftButtonClicked);
	GraphicsRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleGraphicsRightButtonClicked);

	ResolutionLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleResolutionLeftButtonClicked);
	ResolutionRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleResolutionRightButtonClicked);

	RevertButton->OnClicked.RemoveDynamic(this, &USettingsWidget::RevertPendingState);
	ApplyButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleApplyClicked);
}

void USettingsWidget::Render()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	// Update window mode selection.
	const auto FullscreenMode = LexToString(PendingWindowMode);
	WindowModeSelection->SetText(FText::FromString(FullscreenMode));

	// Update VSync selection.
	const auto VSync = PendingVSyncEnabled;
	VSyncSelection->SetText(FText::FromString(VSync ? TEXT("Enabled") : TEXT("Disabled")));

	// Update graphics (quality) selection.
	// Note, the values are -1:custom, 0:low, 1:medium, 2:high, 3:epic, 4:cinematic.
	const auto ScalabilityLevel = PendingGraphicsQuality;
	FString ScalabilityText("");
	switch (ScalabilityLevel)
	{
	case -1:
		ScalabilityText = TEXT("Custom");
		break;
	case 0:
		ScalabilityText = TEXT("Low");
		break;
	case 1:
		ScalabilityText = TEXT("Medium");
		break;
	case 2:
		ScalabilityText = TEXT("High");
		break;
	case 3:
		ScalabilityText = TEXT("Epic");
		break;
	case 4:
		ScalabilityText = TEXT("Cinematic");
		break;
	default:
		check(false);
		break;
	}
	GraphicsSelection->SetText(FText::FromString(ScalabilityText));

	// Update resolution selection.
	const auto ScreenResolution = PendingGraphicsResolution;
	ResolutionSelection->SetText(
		FText::FromString(FString::Printf(TEXT("%d x %d"), ScreenResolution.X, ScreenResolution.Y)));

	UpdateEnabledStateOfActionButtons();
}

void USettingsWidget::UpdateWindowMode(const int NewWindowMode)
{
	// Whenever we change window mode, reset the selected resolution because it may be invalid.
	if (NewWindowMode != PendingWindowMode)
	{
		TArray<FIntPoint> Resolutions;
		if (NewWindowMode == EWindowMode::Fullscreen)
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
		}
		else
		{
			UKismetSystemLibrary::GetConvenientWindowedResolutions(Resolutions);
		}

		PendingGraphicsResolution = Resolutions[0];
	}

	PendingWindowMode = EWindowMode::ConvertIntToWindowMode(NewWindowMode);
	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleWindowModeButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto FullscreenMode = PendingWindowMode == EWindowMode::Fullscreen
		                            ? EWindowMode::Windowed
		                            : EWindowMode::Fullscreen;
	UpdateWindowMode(FullscreenMode);
}

static FORCEINLINE int GetMonitorRefreshRate()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	// ReSharper disable once CppTooWideScopeInitStatement
	FScreenResolutionArray Resolutions;
	if (RHIGetAvailableResolutions(Resolutions, false))
	{
		// Pre-conditions.
		check(Resolutions.Num() > 0);

		const auto MonitorRefreshRate = Resolutions[0].RefreshRate;

		// Post-conditions.
		for (const auto [_, __, RefreshRate] : Resolutions)
		{
			checkf(RefreshRate == MonitorRefreshRate, TEXT("All refresh rates should have been the same."));
		}

		return MonitorRefreshRate;
	}

	checkf(false, TEXT("Monitor refresh rate could not be obtained."));
	return -1;
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleVSyncButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto NewIsSyncEnabledValue = !PendingVSyncEnabled;
	PendingVSyncEnabled = NewIsSyncEnabledValue;

	// Note that 0 disables frame rate limiting, as per the docs:
	// https://docs.unrealengine.com/5.2/en-US/API/Runtime/Engine/GameFramework/UGameUserSettings/SetFrameRateLimit/
	const auto NewFrameRateLimit = NewIsSyncEnabledValue ? GetMonitorRefreshRate() : 0;
	PendingFrameRateLimit = NewFrameRateLimit;

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleGraphicsLeftButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto Scalability = PendingGraphicsQuality;
	if (const auto IsCustom = Scalability == -1)
	{
		// Wrap around from -1 to 4.
		Scalability = 4;
	}
	else
	{
		// 5 scalability levels: low, medium, high, epic, cinematic
		Scalability -= 1;
		Scalability = FMathHelpers::NegativeFriendlyMod(Scalability, 5);
	}

	// Post-conditions.
	check(Scalability != -1);

	PendingGraphicsQuality = Scalability;
	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleGraphicsRightButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto Scalability = PendingGraphicsQuality;
	Scalability += 1;
	Scalability %= 5; // 5 scalability levels: low, medium, high, epic, cinematic
	PendingGraphicsQuality = Scalability;

	Render();
}

void USettingsWidget::HandleResolutionButtonClicked(std::function<int(int, int)> UpdateResolutionIndex)
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	// Get list of supported resolutions depending on window mode.
	TArray<FIntPoint> SupportedResolutions;
	if (PendingWindowMode == EWindowMode::Fullscreen)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);
	}
	else
	{
		UKismetSystemLibrary::GetConvenientWindowedResolutions(SupportedResolutions);
	}

	// Find the new resolution index.
	auto CurrentResolutionIndex = SupportedResolutions.IndexOfByKey(PendingGraphicsResolution);
	const auto NewResolutionIndex = UpdateResolutionIndex(CurrentResolutionIndex, SupportedResolutions.Num());

	// Find the new resolution.
	check(0 <= NewResolutionIndex && NewResolutionIndex < SupportedResolutions.Num());
	const auto NewResolution = SupportedResolutions[NewResolutionIndex];

	PendingGraphicsResolution = NewResolution;
	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleResolutionLeftButtonClicked()
{
	HandleResolutionButtonClicked([](int CurrentResolutionIndex, const int NumResolutions)
	{
		if (CurrentResolutionIndex == INDEX_NONE)
		{
			// Start at zero in preparation for the decrement below.
			CurrentResolutionIndex = 0;
		}

		CurrentResolutionIndex -= 1;
		CurrentResolutionIndex = FMathHelpers::NegativeFriendlyMod(CurrentResolutionIndex, NumResolutions);
		return CurrentResolutionIndex;
	});
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleResolutionRightButtonClicked()
{
	HandleResolutionButtonClicked([](int CurrentResolutionIndex, const int NumResolutions)
	{
		if (CurrentResolutionIndex == INDEX_NONE)
		{
			CurrentResolutionIndex = 0;
		}
		else
		{
			CurrentResolutionIndex += 1;
			CurrentResolutionIndex = FMathHelpers::NegativeFriendlyMod(CurrentResolutionIndex, NumResolutions);
		}
		return CurrentResolutionIndex;
	});
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::UpdateEnabledStateOfActionButtons()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();

	const auto OldWindowMode = GameUserSettings->GetFullscreenMode();
	const auto OldVSyncEnabled = GameUserSettings->IsVSyncEnabled();
	const auto OldGraphicsQuality = GameUserSettings->GetOverallScalabilityLevel();
	const auto OldGraphicsResolution = GameUserSettings->GetScreenResolution();
	const auto OldFrameRateLimit = GameUserSettings->GetFrameRateLimit();

	const bool IsDirty = OldWindowMode != PendingWindowMode
		|| OldVSyncEnabled != PendingVSyncEnabled
		|| OldGraphicsQuality != PendingGraphicsQuality
		|| OldGraphicsResolution != PendingGraphicsResolution
		|| OldFrameRateLimit != PendingFrameRateLimit;

	RevertButton->SetIsEnabled(IsDirty);
	ApplyButton->SetIsEnabled(IsDirty);
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleApplyClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	GameUserSettings->SetFullscreenMode(PendingWindowMode);
	GameUserSettings->SetVSyncEnabled(PendingVSyncEnabled);
	const auto OldGraphicsQuality = GameUserSettings->GetOverallScalabilityLevel();
	GameUserSettings->SetOverallScalabilityLevel(PendingGraphicsQuality);
	GameUserSettings->SetScreenResolution(PendingGraphicsResolution);
	GameUserSettings->SetFrameRateLimit(PendingFrameRateLimit);
	GameUserSettings->SetResolutionScaleNormalized(1.0);

	GameUserSettings->ApplySettings(false);

	// Sometimes this still returns -1 (meaning custom), so let's refresh PendingGraphicsQuality
	// with that knowledge in mind.
	PendingGraphicsQuality = GameUserSettings->GetOverallScalabilityLevel();

	Render();
}
