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

UENUM()
enum class EGhostState : uint8
{
	Normal,
	Frightened,
	Eaten,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostStateChanged,
                                            EGhostState, HasBeenEaten);

// TODO: This class has many responsibilities, see the bottom of this file for potential refactorings when it's expedient.
UCLASS()
class AGhostAiController : public AAIController
{
	GENERATED_BODY()

	// Whether to print out A* map info in the logs.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool DebugAStarMap = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Debug View")
	EGhostState GhostState = EGhostState::Normal;

	UFUNCTION()
	void UpdateWhenSubstateChanges(EChompPlayingSubstateEnum OldState, EChompPlayingSubstateEnum NewState);

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

	bool IsInGhostHouseQueue() const;

	AGhostHouseQueue* GetGhostHouseQueue() const;

	bool IsPlayerAlive() const;

	// A setter that will also notify observers of the changed internal state.
	void SetGhostState(const EGhostState NewGhostState);

	UFUNCTION()
	void HandleGhostStateChanged(const EGhostState NewGhostState);

	FMovementPath ReturnToGhostHouse() const;

	FGridLocation ComputeDestinationNodeInFrightened(const FGridLocation& GridLocation) const;

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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual FGridLocation GetPlayerGridLocation() const;

	virtual FGridLocation GetPlayerGridDirection() const;

	virtual FVector GetPlayerWorldLocation() const;

	FMovementPath UpdateMovementPathWhenInChase() const;

	FMovementPath UpdateMovementPathWhenInScatter();

	FMovementPath UpdateMovementPathWhenInFrightened() const;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnGhostStateChanged OnGhostStateChanged;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	FGridLocation GetChaseEndGridPosition() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Customizable AI Behavior")
	void DecideToUpdateMovementPathInChase(FVector NewLocation);

	virtual FGridLocation GetChaseEndGridPosition_Implementation() const;

	virtual void DecideToUpdateMovementPathInChase_Implementation(FVector NewLocation);

	void HandleGameStateChanged(EChompGameStateEnum OldState, EChompGameStateEnum NewState);

	int GetLeaveGhostHousePriority() const;

	UFUNCTION(BlueprintCallable)
	bool IsEaten() const;

	UFUNCTION(BlueprintCallable)
	bool IsNormal() const;

	void Consume();
};

#if false

* GhostAiController (the brain of the Ghost pawn)
	* GhostState (field)
	* UpdateWhenSubstateChanged
	* ResetGhostState (emit an event)
	* SetGhostState
	* HandleGhostStateChanged
	* OnPossess
	* Tick()
	* OnGhostStateChanged
	* HandleGameStateChanged
	* IsEaten
	* Consume
* Member of ghosthousequeue (component)
	* HandleDotsConsumedUpdated
	* ResetGhostState
	* IsInGhostHouseQueue
	* GetGhostHouseQueue
	* IsStartingPositionInGhostHouse (helper)
	* GetLeaveGhostHousePriority
* A* maze navigator component
	* ComputePath
	* DebugAStarMap (config), whether to print out map
	* DebugAStar
	* ReturnToGhostHouse
	* MovementPath (field)
	* CurrentScatterOrigin
	* CurrentScatterDestination
	* Tick()
	* Possibly use state pattern:
		* UpdateMovementPathWhenInChase
		* UpdateMovementPathWhenInScatter
		* UpdateMovementPathWhenInFrightened
		* GetChaseEndGridPosition
		* DecideToUpdateMovementPathInChase
		* GetChaseEndGridPosition_Implementation
		* DecideToUpdateMovementPathInChase_Implementation
* Util (FSafeGet)
	* IsPlayerAlive
* ChompPlayerController
	* GetPlayerGridLocation (static)
	* GetPlayerGridDirection (static)
	* GetPlayerWorldLocation (static)

#endif
