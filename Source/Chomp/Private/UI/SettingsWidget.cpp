#include "SettingsWidget.h"

#include "DynamicRHI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameUserSettings.h"
#include "Utils/Debug.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Render();

	WindowModeLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeLeftButtonClicked);
	WindowModeRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleWindowModeRightButtonClicked);

	VSyncLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);
	VSyncRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleVSyncButtonClicked);

	GraphicsLeftButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsLeftButtonClicked);
	GraphicsRightButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleGraphicsRightButtonClicked);

	ApplyButton->OnClicked.AddUniqueDynamic(this, &USettingsWidget::HandleApplyClicked);
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
	
	ApplyButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleApplyClicked);
}

void USettingsWidget::Render() const
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto FullscreenMode = LexToString(GameUserSettings->GetFullscreenMode());
	WindowModeSelection->SetText(FText::FromString(FullscreenMode));

	const auto VSync = GameUserSettings->IsVSyncEnabled();
	VSyncSelection->SetText(FText::FromString(VSync ? TEXT("Enabled") : TEXT("Disabled")));

	// @param Value -1:custom, 0:low, 1:medium, 2:high, 3:epic, 4:cinematic
	const auto ScalabilityLevel = GameUserSettings->GetOverallScalabilityLevel();
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

	const auto ScreenResolution = GameUserSettings->GetScreenResolution();
	ResolutionSelection->SetText(
		FText::FromString(FString::Printf(TEXT("%d x %d"), ScreenResolution.X, ScreenResolution.Y)));
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleWindowModeLeftButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto FullscreenMode = static_cast<int>(GameUserSettings->GetFullscreenMode());
	FullscreenMode -= 1;
	FullscreenMode %= EWindowMode::NumWindowModes;
	GameUserSettings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(FullscreenMode));

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleWindowModeRightButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto FullscreenMode = static_cast<int>(GameUserSettings->GetFullscreenMode());
	FullscreenMode += 1;
	FullscreenMode %= EWindowMode::NumWindowModes;
	GameUserSettings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(FullscreenMode));

	Render();
}

static int GetMonitorRefreshRate()
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

	const auto NewIsSyncEnabledValue = !GameUserSettings->IsVSyncEnabled();
	GameUserSettings->SetVSyncEnabled(NewIsSyncEnabledValue);

	// Note that 0 disables frame rate limiting, as per the docs:
	// https://docs.unrealengine.com/5.2/en-US/API/Runtime/Engine/GameFramework/UGameUserSettings/SetFrameRateLimit/
	const auto NewFrameRateLimit = NewIsSyncEnabledValue ? GetMonitorRefreshRate() : 0;
	GameUserSettings->SetFrameRateLimit(GetMonitorRefreshRate());

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleGraphicsLeftButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto Scalability = GameUserSettings->GetOverallScalabilityLevel();
	if (const auto IsCustom = Scalability == -1)
	{
		Scalability = 4;
	}
	else
	{
		Scalability -= 1;
		Scalability %= 5; // 5 scalability levels: low, medium, high, epic, cinematic
	}
	GameUserSettings->SetOverallScalabilityLevel(Scalability);

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void USettingsWidget::HandleGraphicsRightButtonClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	auto Scalability = GameUserSettings->GetOverallScalabilityLevel();
	Scalability += 1;
	Scalability %= 5; // 5 scalability levels: low, medium, high, epic, cinematic
	GameUserSettings->SetOverallScalabilityLevel(Scalability);

	Render();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeStatic
void USettingsWidget::HandleApplyClicked()
{
	// Pre-conditions.
	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);
	
	GameUserSettings->SetResolutionScaleNormalized(1.0);
	GameUserSettings->ApplySettings(false);
}
