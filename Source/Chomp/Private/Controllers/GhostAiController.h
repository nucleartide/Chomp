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

UCLASS()
class AGhostAiController : public AAIController
{
	GENERATED_BODY()

	// Whether to print out A* map info in the logs.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool DebugAStarMap = false;

	// Whether we are currently running test code for the sake of debugging.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool IsTesting = false;

	UPROPERTY(VisibleAnywhere)
	FMovementPath MovementPath;

	UPROPERTY(VisibleAnywhere)
	FGridLocation CurrentScatterOrigin;

	UPROPERTY(VisibleAnywhere)
	FGridLocation CurrentScatterDestination;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

public:
	void HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState);

	int GetLeaveGhostHousePriority() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleGamePlayingSubstateChanged(EChompGamePlayingSubstate OldState, EChompGamePlayingSubstate NewState);
	
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

	FMovementPath UpdateMovementPathWhenInScatter() const;

	FMovementPath UpdateMovementPathWhenInChase() const;

	void ResetGhostState();

	bool IsStartingPositionInGhostHouse() const;

	bool IsInGhostHouse() const;
	
	AGhostHouseQueue* GetGhostHouseQueue() const;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	FGridLocation GetChaseEndGridPosition() const;
	
	virtual FGridLocation GetChaseEndGridPosition_Implementation() const;
};
