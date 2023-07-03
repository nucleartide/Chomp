#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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

	void MoveFrom(FVector2D Source, FVector2D Destination);
	void Move(float DeltaTime);
	void StartMovingFrom(FVector2D Origin, FVector2D Destination);

	FVector2D CurrentOrigin;
	FVector2D CurrentDestination;
	bool IsAtDestination = false;
	float ElapsedTime = 0.0f;
	float TotalTime = 0.0f;

	virtual void BeginPlay() override;
};
