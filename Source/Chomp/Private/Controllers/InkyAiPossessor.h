#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InkyAiPossessor.generated.h"

class AGhostAiController;
class AGhostPawn;

// This class is only needed because we'd like to obtain a reference to BlinkyPawn,
// while not having to reach out into the scene to do so.
//
// Thus with this class, you can dependency inject a reference to BlinkyPawn at edit time
// so we don't need to fetch that reference at run time.
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
