#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AStar/GridLocation.h"
#include "ChompPlayerController.generated.h"

UCLASS()
class AChompPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

private:
	float VerticalAxisInput;
	float HorizontalAxisInput;
	FGridLocation TargetTile{0, 0};

	/**
	 * A flag to denote whether TargetTile is currently set.
	 *
	 * This is necessary because FGridLocation is a struct, and thus there is no "null" value.
	 */
	bool IsTargetTileSet = false;

public:
	AChompPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void OnMoveHorizontal(float Delta);
	void OnMoveVertical(float Delta);
};
