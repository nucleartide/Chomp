#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PacmanPawnManager.generated.h"

/**
 * APacmanPawnManager defines the starting location of the player pawn,
 * and resets the player pawn's position when the game is restarted.
 */
UCLASS()
class APacmanPawnManager : public AActor
{
	GENERATED_BODY()

public:
	APacmanPawnManager();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleGameRestarted();

	UFUNCTION()
	void HandlePacmanDied();

	UPROPERTY(EditAnywhere)
	class APacmanPawn *PacmanPawn;

public:
	virtual void Tick(float DeltaTime) override;
};
