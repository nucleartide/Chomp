#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "MyDeveloperSettings.generated.h"

class ULevelDataAsset;

UCLASS(config=Game, defaultconfig, meta=(DisplayName="My Settings"))
class CHOMP_API UMyDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	ULevelDataAsset* HighScoreLevel;
};
