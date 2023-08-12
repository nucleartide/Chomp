#pragma once

#include "CoreMinimal.h"
#include "ChompGameState.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "AStar/GridLocation.h"
#include "ChompPawnManager.generated.h"

/**
 * AChompPawnManager defines the starting location of the player pawn,
 * and resets the player pawn's position when the game is restarted.
 */
UCLASS()
class AChompPawnManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<class AChompPawn> ChompPawn;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation SpawnGridPosition;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FRotator SpawnRotation;

	TWeakObjectPtr<AChompPawn> ChompPawnInstance;
	
public:
	UFUNCTION()
	void HandleGameRestarted(EChompGameStateEnum OldState, EChompGameStateEnum NewState);

protected:
	virtual void BeginPlay() override;
};
