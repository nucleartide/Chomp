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

private:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool UseTestOriginAndDestination = false;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation TestOrigin;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation TestDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation ScatterOrigin;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation ScatterDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool DebugAStarMap = false;

private:
	// Similar to AChompPlayerController, we store a notion of a TargetTile.
	FComputeTargetTileResult Target;

	// However, the concepts of "CurrentMoveDirection" and "IntendedMoveDirection" are encapsulated into the AI's computed movement path.
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

	void ComputeScatterForMovementPath();

	void ComputeChaseForMovementPath();
};
