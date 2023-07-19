#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChompPlayerController.generated.h"

UCLASS()
class AChompPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AChompPlayerController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void OnMoveHorizontal(float Delta);
	void OnMoveVertical(float Delta);

	float VerticalAxisDelta;
	float HorizontalAxisDelta;

	enum class Axis : uint8
	{
		None,
		Vertical,
		Horizontal,
	};

	Axis LastAxisToBecomeNonZero = Axis::None;

	UPROPERTY(EditAnywhere, Category = "Player Controller Customization")
	float MovementSpeed = 3.0f;
};
