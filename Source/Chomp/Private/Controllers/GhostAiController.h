#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "AIController.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"
#include "AStar/MovementPath.h"
#include "LevelGenerator/LevelLoader.h"
#include "GhostAiController.generated.h"

class AGhostHouseQueue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHasBeenEatenChanged,
                                            bool, HasBeenEaten);

UCLASS()
class AGhostAiController : public AAIController
{
	GENERATED_BODY()

	// Whether to print out A* map info in the logs.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool DebugAStarMap = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Debug View")
	bool HasBeenEaten = false;

	UFUNCTION()
	void HandleGamePlayingSubstateChanged(EChompPlayingSubstateEnum OldState, EChompPlayingSubstateEnum NewState);

	UFUNCTION()
	void HandleDotsConsumedUpdated(int NewDotsConsumed);

	static TArray<FGridLocation> ComputePath(
		ULevelLoader* LevelInstance,
		FVector2D CurrentWorldPosition,
		FGridLocation StartGridPos,
		FGridLocation EndGridPos,
		bool Debug
	);

	static void DebugAStar(
		const std::unordered_map<FGridLocation, FGridLocation>& CameFrom,
		ULevelLoader* LevelInstance
	);

	void ResetGhostState();

	bool IsStartingPositionInGhostHouse() const;

	bool IsInGhostHouse() const;

	AGhostHouseQueue* GetGhostHouseQueue() const;

	bool IsPlayerAlive() const;

	// A setter that will also notify observers of the changed internal state.
	void SetHasBeenEaten(bool WasJustEaten);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	UPROPERTY(VisibleInstanceOnly)
	FMovementPath MovementPath;
	
	UPROPERTY(VisibleInstanceOnly)
	FGridLocation CurrentScatterOrigin;

	UPROPERTY(VisibleInstanceOnly)
	FGridLocation CurrentScatterDestination;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual FGridLocation GetPlayerGridLocation() const;

	virtual FGridLocation GetPlayerGridDirection() const;

	virtual FVector GetPlayerWorldLocation() const;

	FMovementPath UpdateMovementPathWhenInChase() const;
	
	FMovementPath UpdateMovementPathWhenInScatter();

	FMovementPath UpdateMovementPathWhenInFrightened() const;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHasBeenEatenChanged OnHasBeenEatenChanged;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	FGridLocation GetChaseEndGridPosition() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	void DecideToUpdateMovementPathInChase(FVector NewLocation);

	virtual FGridLocation GetChaseEndGridPosition_Implementation() const;
	
	virtual void DecideToUpdateMovementPathInChase_Implementation(FVector NewLocation);

	void HandleGameStateChanged(EChompGameStateEnum OldState, EChompGameStateEnum NewState);

	int GetLeaveGhostHousePriority() const;

	bool GetHasBeenEaten() const;

	void Consume();
};
