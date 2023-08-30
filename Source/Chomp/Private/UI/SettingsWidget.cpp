#include "SettingsWidget.h"

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

	// TODO: remember to set these remaining fields.
#if false
	const auto FrameRateLimit = GameUserSettings->GetFrameRateLimit();
	const auto ResScaleNormalized = GameUserSettings->GetResolutionScaleNormalized();
	const auto ScreenResolution = GameUserSettings->GetScreenResolution();
	DEBUG_LOG(TEXT("test"));
#endif
}

void USettingsWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	WindowModeLeftButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeLeftButtonClicked);
	WindowModeRightButton->OnClicked.RemoveDynamic(this, &USettingsWidget::HandleWindowModeRightButtonClicked);
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
	ResolutionSelection->SetText(FText::FromString(FString::Printf(TEXT("%d x %d"), ScreenResolution.X, ScreenResolution.Y)));
}

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
