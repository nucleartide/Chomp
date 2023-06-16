#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PacmanPlayerController.generated.h"

UCLASS()
class APacmanPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APacmanPlayerController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void OnMoveHorizontal(float Delta);
	void OnMoveVertical(float Delta);

	float VerticalAxisDelta;
	float HorizontalAxisDelta;

	UPROPERTY(EditAnywhere, Category = "Player Controller Customization")
	float MovementSpeed = 3.0f;
};
