#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InkyAiPossessor.generated.h"

class AGhostAiController;
class AGhostPawn;

UCLASS()
class CHOMP_API AInkyAiPossessor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGhostAiController> InkyAiController;

	UPROPERTY(EditInstanceOnly)
	AGhostPawn* BlinkyPawn;

	UPROPERTY(EditInstanceOnly)
	AGhostPawn* InkyPawn;

protected:
	virtual void BeginPlay() override;
};
