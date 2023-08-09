#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "InkyAiController.generated.h"

class AGhostPawn;

UCLASS()
class CHOMP_API AInkyAiController : public AGhostAiController
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	AGhostPawn* BlinkyPawnRef;
	
protected:
	virtual FGridLocation GetChaseEndGridPosition_Implementation() const override;
	
public:
	void Initialize(AGhostPawn* BlinkyPawn);
};
