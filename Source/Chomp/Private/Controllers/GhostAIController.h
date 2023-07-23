#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"

#include "AIController.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"

#include "GhostAIController.generated.h"

struct Path
{
private:
	int CurrentLocation = 0;
	std::vector<FGridLocation> Locations;

public:
	void Initialize(const std::vector<FGridLocation>& NewLocations)
	{
		CurrentLocation = 0;
		Locations = NewLocations;
	}

	FGridLocation GetCurrentLocation()
	{
		return Locations.at(CurrentLocation);
	}

	FGridLocation GetTargetLocation()
	{
		return Locations.at(CurrentLocation + 1);
	}
};

UCLASS()
class AGhostAIController : public AAIController
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

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
	Path MovementPath;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void HandleGamePlayingSubstateChanged(EChompGamePlayingSubstate OldState, EChompGamePlayingSubstate NewState);

	UFUNCTION()
	void HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState);

private:
	static std::vector<FGridLocation> ComputePath(
		ULevelLoader* LevelInstance,
		FVector2D CurrentWorldPosition,
		FGridLocation OriginGridPosition,
		FGridLocation DestinationGridPosition,
		bool DebugAStarMap);
	bool CanStartMoving() const;
	static void DebugAStar(const std::unordered_map<FGridLocation, FGridLocation> &CameFrom, ULevelLoader *LevelInstance);
	void ComputeScatterPath();
	void ComputeChasePath();
};
