#include "ChompGameMode.h"

#include "GameFramework/GameUserSettings.h"

void AChompGameMode::BeginPlay()
{
	Super::BeginPlay();

	/*
	const auto GameUserSettings = UGameUserSettings::GetGameUserSettings();
	GameUserSettings->SetOverallScalabilityLevel(0);
	GameUserSettings->SetScreenResolution(GameUserSettings->GetDesktopResolution());
	GameUserSettings->SetResolutionScaleNormalized(1.0);
	GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
	GameUserSettings->ApplySettings(false);
	*/

	// TODO: where is this save file being written? that'll help me figure out what's going on
	// TODO: port to C++
	// TODO: wrap up this PR
	// TODO: break
}
