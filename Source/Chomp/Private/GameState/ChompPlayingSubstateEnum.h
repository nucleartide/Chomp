#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChompPlayingSubstateEnum.generated.h"

/**
 * When in the EChompGameStateEnum::Playing state, these would be the sub-states.
 */
UENUM()
enum class EChompPlayingSubstateEnum : uint8
{
	None,
	Scatter,
	Chase,
	Frightened,
};
