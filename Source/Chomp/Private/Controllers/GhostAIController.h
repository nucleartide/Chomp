#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"

#include "AIController.h"
#include "AStar/GridLocation.h"
#include "ChompGameState.h"

#include "GhostAIController.generated.h"

struct Path
{
	int CurrentIndex = 0;
	std::vector<FGridLocation> Locations;

	void Initialize(const std::vector<FGridLocation>& NewLocations)
	{
		CurrentIndex = 0;
		Locations = NewLocations;
	}
};

UCLASS()
class AGhostAIController : public AAIController
{
	GENERATED_BODY()

	/**
	 * Fields.
	 */

private:
	Path CurrentPath;
	FGridLocation CurrentOriginGridPos;
	FGridLocation CurrentDestinationGridPos;
	bool IsAtDestination = false;

	/**
	 * Indicates whether a ghost has started moving, or is currently idle in the ghosthouse.
	 */
	bool DidStartMoving = false;

	/**
	 * Properties.
	 */

private:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool IsTestOriginAndDestinationEnabled = false;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

	// Note: this is only used for testing.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation Origin;

	// Note: this is only used for testing.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation Destination;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation ScatterOrigin;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation ScatterDestination;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool Debug = false;

	/**
	 * Callbacks.
	 */

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/**
	 * Behavior.
	 */

private:
	void StartMovingFrom(FGridLocation Origin, FGridLocation Destination);
	void MoveTowardDestination(float DeltaTime);
	void DebugAStar(std::unordered_map<FGridLocation, FGridLocation> &CameFrom);
	void HandleScatterNodeReached();
	void HandleChaseNodeReached();
	void Scatter();
	void Chase();

	UFUNCTION()
	void HandleGamePlayingStateChanged(EChompGamePlayingState OldState, EChompGamePlayingState NewState);

	UFUNCTION()
	void HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState);

	UFUNCTION()
	void HandleDotsConsumedChanged(int NumberOfDotsConsumed);
};
