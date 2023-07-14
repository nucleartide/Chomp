#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"

#include "AIController.h"
#include "AStar/GridLocation.h"

#include "GhostAIController.generated.h"

struct Path
{
	int CurrentIndex = 0;
	std::vector<FGridLocation> Locations;

	void Initialize(const std::vector<FGridLocation>& NewLocations)
	{
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
	 * Properties.
	 */

private:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	bool IsTestOriginAndDestinationEnabled = false;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation Origin;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation Destination;

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
	void Move(float DeltaTime);
	void DebugAStar(std::unordered_map<FGridLocation, FGridLocation> &CameFrom);
};
