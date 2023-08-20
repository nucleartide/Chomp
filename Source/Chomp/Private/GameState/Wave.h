#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "UObject/Object.h"
#include "Wave.generated.h"

/**
 * An FWave is a period of time during which a particular EChompPlayingSubstateEnum is active.
 */
USTRUCT()
struct FWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EChompPlayingSubstateEnum PlayingState = EChompPlayingSubstateEnum::None;

	UPROPERTY(EditAnywhere)
	double Duration = 0.0;
};
