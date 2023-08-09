#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "ClydeAiController.generated.h"

UCLASS()
class CHOMP_API AClydeAiController : public AGhostAiController
{
	GENERATED_BODY()

	// If Clyde's distance to Pacman is < CentimetersAwayFromPacman, Clyde pursues Pacman.
	// Else, Clyde retreats to their corner.
	UPROPERTY(EditDefaultsOnly)
	double CentimetersAwayFromPacman = 800.0;
	
	UPROPERTY(VisibleInstanceOnly)
	bool WasChasingPacman = false;
	
	UPROPERTY(VisibleInstanceOnly)
	bool ShouldChasePacman = false;
	
protected:
	virtual FGridLocation GetChaseEndGridPosition_Implementation() const override;
	
	virtual void DecideToUpdateMovementPathInChase_Implementation(FVector NewLocation) override;
};
