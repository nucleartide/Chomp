#pragma once

#include "CoreMinimal.h"
#include "Math/IntPoint.h"

#include "AIController.h"
#include "AStar/GridLocation.h"

#include "GhostAIController.generated.h"

UCLASS()
class AGhostAIController : public AAIController
{
	GENERATED_BODY()

private:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "AI Controller Customization")
	float MovementSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI Controller Customization")
	bool IsTestOriginAndDestinationEnabled = false;

	UPROPERTY(EditDefaultsOnly, Category = "AI Controller Customization")
	TSubclassOf<class ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "AI Controller Customization")
	FGridLocation Origin;

	UPROPERTY(EditDefaultsOnly, Category = "AI Controller Customization")
	FGridLocation Destination;

	void StartMovingFrom(FGridLocation Origin, FGridLocation Destination);
	void Move(float DeltaTime);
	void Pathfind(FGridLocation Destination);

	FGridLocation CurrentOriginGridPos;
	FGridLocation CurrentDestinationGridPos;

	bool IsAtDestination = false;
	float ElapsedTime = 0.0f;
	float TotalTime = 0.0f;

	virtual void BeginPlay() override;

	struct Path
	{
		int CurrentIndex = 0;
		std::vector<FGridLocation> Locations;

	    // Copy assignment operator
		Path& operator=(const Path& other) {
			if (this != &other) {
				// Copy the data from the other object
				CurrentIndex = other.CurrentIndex;
				Locations = other.Locations;
			}
			// Return the reference to the current object
			return *this;
		}
	};

	Path CurrentPath;
};