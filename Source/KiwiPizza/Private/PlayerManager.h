#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerManager.generated.h"

/**
 * APlayerManager defines the starting location of the player pawn,
 * and resets the player pawn's position when the game is restarted.
 */
UCLASS()
class APlayerManager : public AActor
{
	GENERATED_BODY()

public:
	APlayerManager();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleGameRestarted();

	UPROPERTY(EditAnywhere)
	class APacmanPawn *PacmanPawn;

public:
	virtual void Tick(float DeltaTime) override;
};
