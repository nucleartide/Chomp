#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Math/IntPoint.h"
#include "GridLocation.h"
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

	void StartMovingFrom(GridLocation Origin, GridLocation Destination);
	void Move(float DeltaTime);
	void Pathfind(GridLocation Destination);

	GridLocation CurrentOriginGridPos;
	GridLocation CurrentDestinationGridPos;

	bool IsAtDestination = false;
	float ElapsedTime = 0.0f;
	float TotalTime = 0.0f;

	virtual void BeginPlay() override;
};
