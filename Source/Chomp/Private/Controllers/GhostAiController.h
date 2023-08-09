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

	UPROPERTY(VisibleAnywhere)
	FMovementPath MovementPath;

	UPROPERTY(VisibleAnywhere)
	FGridLocation CurrentScatterOrigin;

	UPROPERTY(VisibleAnywhere)
	FGridLocation CurrentScatterDestination;

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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual FGridLocation GetPlayerGridLocation() const;

	virtual FGridLocation GetPlayerGridDirection() const;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	FGridLocation GetChaseEndGridPosition() const;

	virtual FGridLocation GetChaseEndGridPosition_Implementation() const;
	
	void HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState);

	int GetLeaveGhostHousePriority() const;
};
