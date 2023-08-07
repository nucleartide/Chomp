#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "InkyAiController.generated.h"

class AGhostPawn;

UCLASS()
class CHOMP_API AInkyAiController : public AGhostAiController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	AGhostPawn* BlinkyPawnRef;

public:
	void Initialize(AGhostPawn* BlinkyPawn);

	virtual FMaybeGridLocation GetChaseStartGridPosition_Implementation() const override;

	virtual FMaybeGridLocation GetChaseEndGridPosition_Implementation() const override;

private:
	FMaybeGridLocation GetPlayerGridLocation() const;
};
