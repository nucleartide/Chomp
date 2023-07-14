#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "ChompPawnManager.generated.h"

/**
 * AChompPawnManager defines the starting location of the player pawn,
 * and resets the player pawn's position when the game is restarted.
 */
UCLASS()
class AChompPawnManager : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<class AChompPawn> ChompPawnInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AChompPawn> ChompPawn;

	UFUNCTION()
	void HandleGameRestarted(EChompGameState OldState, EChompGameState NewState);
};
