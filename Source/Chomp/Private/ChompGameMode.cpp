#include "ChompGameMode.h"
#include "GameFramework/GameUserSettings.h"

void AChompGameMode::BeginPlay()
{
	Super::BeginPlay();

	const auto GameUserSettings = UGameUserSettings::GetGameUserSettings();
	GameUserSettings->SetOverallScalabilityLevel(1);
	GameUserSettings->SetScreenResolution(GameUserSettings->GetDesktopResolution());
	GameUserSettings->SetResolutionScaleNormalized(1.0);
	GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
	GameUserSettings->ApplySettings(false);
}
