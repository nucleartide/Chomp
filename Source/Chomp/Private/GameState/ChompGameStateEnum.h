#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChompGameStateEnum.generated.h"

UENUM(BlueprintType)
enum class EChompGameStateEnum : uint8
{
	None,
	Playing,
	GameOverWin,
	GameOverLose,
};
