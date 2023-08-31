#include "SettingsWidget.h"

#include <functional>
#include "DynamicRHI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utils/Debug.h"
#include "Utils/MathHelpers.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WindowModeLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeLeftButtonClicked);
	WindowModeRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeRightButtonClicked);

	VSyncLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);
	VSyncRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);

	GraphicsLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsLeftButtonClicked);
	GraphicsRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsRightButtonClicked);

	ResolutionLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleResolutionLeftButtonClicked);
	ResolutionRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleResolutionRightButtonClicked);

	ApplyButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleApplyClicked);

	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();

	// Initialize component state.
	PendingWindowMode = GameUserSettings->GetFullscreenMode();
	PendingVSyncEnabled = GameUserSettings->IsVSyncEnabled();
	PendingGraphicsQuality = GameUserSettings->GetOverallScalabilityLevel();
	PendingGraphicsResolution = GameUserSettings->GetScreenResolution();
	PendingFrameRateLimit = GameUserSettings->GetFrameRateLimit();

	Render();
}

void USettingsWidget::NativeDestruct()
{
	Super::NativeDestruct();

	WindowModeLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeLeftButtonClicked);
	WindowModeRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeRightButtonClicked);

	VSyncLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);
	VSyncRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);

	GraphicsLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleGraphicsLeftButtonClicked);
	GraphicsRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleGraphicsRightButtonClicked);

	ResolutionLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleResolutionLeftButtonClicked);
	ResolutionRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleResolutionRightButtonClicked);

	ApplyButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleApplyClicked);
}

void USettingsWidget::Render() const
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto FullscreenMode = LexToString(PendingWindowMode);
	WindowModeSelection->SetText(FText::FromString(FullscreenMode));

	const auto VSync = PendingVSyncEnabled;
	VSyncSelection->SetText(FText::FromString(VSync ? TEXT("Enabled") : TEXT("Disabled")));

	// @param Value -1:custom, 0:low, 1:medium, 2:high, 3:epic, 4:cinematic
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

	const auto ScreenResolution = PendingGraphicsResolution;
	ResolutionSelection->SetText(
		FText::FromString(FString::Printf(TEXT("%d x %d"), ScreenResolution.X, ScreenResolution.Y)));
}

void USettingsWidget::UpdateFullscreenMode(int NewFullscreenMode)
{
	// If the new fullscreen mode is different than existing,
	if (NewFullscreenMode != PendingWindowMode)
	{
		// Fetch resolutions based on new window mode.
		TArray<FIntPoint> Resolutions;
		if (NewFullscreenMode == EWindowMode::Fullscreen)
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
		}
		else
		{
			UKismetSystemLibrary::GetConvenientWindowedResolutions(Resolutions);
		}

		// Reset pending screen resolution.
		PendingGraphicsResolution = Resolutions[0];
	}

	// Finally, save out the new fullscreen mode.
	PendingWindowMode = EWindowMode::ConvertIntToWindowMode(NewFullscreenMode);

	// TODO: gotta make same adjustments to right button clicked
	// ...

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleWindowModeLeftButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto FullscreenMode = static_cast<int>(PendingWindowMode);
	FullscreenMode -= 1;
	FullscreenMode %= EWindowMode::NumWindowModes;

	UpdateFullscreenMode(FullscreenMode);
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleWindowModeRightButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto FullscreenMode = static_cast<int>(PendingWindowMode);
	FullscreenMode += 1;
	FullscreenMode %= EWindowMode::NumWindowModes;

	UpdateFullscreenMode(FullscreenMode);
}

static FORCEINLINE int GetMonitorRefreshRate()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto ScreenResolution = GameUserSettings->GetScreenResolution();

	FScreenResolutionArray Resolutions;
	if (RHIGetAvailableResolutions(Resolutions, false))
	{
		// Pre-conditions.
		check(Resolutions.Num() > 0);
		const auto ExpectedRefreshRate = Resolutions[0].RefreshRate;
		for (const auto [_, __, RefreshRate] : Resolutions)
		{
			check(RefreshRate == ExpectedRefreshRate);
		}

		return ExpectedRefreshRate;
	}

	DEBUG_LOGERROR(TEXT("Screen Resolutions could not be obtained"));
	check(false);
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
		Scalability = 4;
	}
	else
	{
		// 5 scalability levels: low, medium, high, epic, cinematic
		Scalability -= 1;
		Scalability = FMathHelpers::NegativeFriendlyMod(Scalability, 5);
		check(Scalability != -1);
	}
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
	// TODO: use negative friendly mod here
	PendingGraphicsQuality = Scalability;

	Render();
}

void USettingsWidget::HandleResolutionButtonClicked(std::function<int(int, int)> SomeFunction)
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	// Get list of supported resolutions depending on screen mode.
	TArray<FIntPoint> SupportedResolutions;
	if (PendingWindowMode == EWindowMode::Fullscreen)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);
	}
	else
	{
		UKismetSystemLibrary::GetConvenientWindowedResolutions(SupportedResolutions);
	}

	// Get the current resolution's index within the supported resolutions.
	auto CurrentResolutionIndex = SupportedResolutions.IndexOfByKey(PendingGraphicsResolution);

	const auto NewResolutionIndex = SomeFunction(CurrentResolutionIndex, SupportedResolutions.Num());

#if false

#endif

	// Find the new resolution.
	check(0 <= NewResolutionIndex && NewResolutionIndex < SupportedResolutions.Num());
	const auto NewResolution = SupportedResolutions[NewResolutionIndex];

	// Assign the new resolution to the PendingResolution field.
	PendingGraphicsResolution = NewResolution;

	// Re-render.
	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleResolutionLeftButtonClicked()
{
	HandleResolutionButtonClicked([](int CurrentResolutionIndex, const int NumResolutions)
	{
		// Decrement the index while using negative friendly mod.
		if (CurrentResolutionIndex == INDEX_NONE)
		{
			// Start at zero, in preparation for the decrement below.
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

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeStatic
void USettingsWidget::HandleApplyClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	GameUserSettings->SetFullscreenMode(PendingWindowMode);
	GameUserSettings->SetVSyncEnabled(PendingVSyncEnabled);
	GameUserSettings->SetOverallScalabilityLevel(PendingGraphicsQuality);
	GameUserSettings->SetScreenResolution(PendingGraphicsResolution);
	GameUserSettings->SetFrameRateLimit(PendingFrameRateLimit);
	GameUserSettings->SetResolutionScaleNormalized(1.0);
	DEBUG_MOVE(TEXT("pending graphics resolution %d %d"), PendingGraphicsResolution.X, PendingGraphicsResolution.Y);
	GameUserSettings->ApplySettings(false);
}
