#include "SettingsWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Utils/Debug.h"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const auto GameUserSettings = GEngine->GetGameUserSettings();
	check(GameUserSettings);

	const auto FullscreenMode = GameUserSettings->GetFullscreenMode();
	const auto VSync = GameUserSettings->IsVSyncEnabled();
	const auto FrameRateLimit = GameUserSettings->GetFrameRateLimit();
	const auto ScalabilityLevel = GameUserSettings->GetOverallScalabilityLevel();
	const auto ResScaleNormalized = GameUserSettings->GetResolutionScaleNormalized();
	const auto ScreenResolution = GameUserSettings->GetScreenResolution();
	DEBUG_LOG(TEXT("test"));
}
