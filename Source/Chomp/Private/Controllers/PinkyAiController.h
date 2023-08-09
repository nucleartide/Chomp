#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "PinkyAiController.generated.h"

UCLASS()
class CHOMP_API APinkyAiController : public AGhostAiController
{
	GENERATED_BODY()

protected:
	virtual FGridLocation GetChaseEndGridPosition_Implementation() const override;
};
