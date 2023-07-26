#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"
#include "AIController.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"
#include "AStar/Path.h"
#include "LevelGenerator/LevelLoader.h"
#include "GhostAIController.generated.h"

UCLASS()
class AGhostAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	// Whether to print out A* map info in the logs.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool DebugAStarMap = false;

	// Whether we are currently running test code for the sake of debugging.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool IsTesting = false;

	FPath MovementPath;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void HandleGamePlayingSubstateChanged(EChompGamePlayingSubstate OldState, EChompGamePlayingSubstate NewState);

	UFUNCTION()
	void HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState);

	static std::vector<FGridLocation> ComputePath(
		ULevelLoader* LevelInstance,
		FVector2D CurrentWorldPosition,
		FGridLocation StartGridPos,
		FGridLocation EndGridPos,
		bool Debug);

	bool CanStartMoving() const;

	static void DebugAStar(const std::unordered_map<FGridLocation, FGridLocation>& CameFrom,
	                       ULevelLoader* LevelInstance);

	void ComputeScatterForMovementPath(const FGridLocation& ScatterDestination);

	void ComputeChaseForMovementPath();
};
