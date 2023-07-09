#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChompPawnManager.generated.h"

/**
 * AChompPawnManager defines the starting location of the player pawn,
 * and resets the player pawn's position when the game is restarted.
 */
UCLASS()
class AChompPawnManager : public AActor
{
	GENERATED_BODY()

public:
	AChompPawnManager();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleGameRestarted();

	UFUNCTION()
	void HandlePacmanDied();

	UPROPERTY(EditAnywhere)
	class AChompPawn *ChompPawn;

public:
	virtual void Tick(float DeltaTime) override;
};
