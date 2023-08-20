#pragma once

#include "CoreMinimal.h"
#include "ChompPlayingSubstate.h"
#include "UObject/Object.h"
#include "Wave.generated.h"

/**
 * An FWave is a period of time during which a particular EChompPlayingSubstateEnum is active.
 */
USTRUCT()
struct FWave
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	EChompPlayingSubstateEnum PlayingState = EChompPlayingSubstateEnum::None;

	UPROPERTY(EditDefaultsOnly)
	double Duration = 0.0;
};
