#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Math/IntPoint.h"
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

	using GridPosition = FIntPoint;

	void StartMovingFrom(GridPosition Origin, GridPosition Destination);
	void Move(float DeltaTime);
	void Pathfind(GridPosition Destination);

	GridPosition CurrentOriginGridPos;
	GridPosition CurrentDestinationGridPos;

	bool IsAtDestination = false;
	float ElapsedTime = 0.0f;
	float TotalTime = 0.0f;

	virtual void BeginPlay() override;
};
