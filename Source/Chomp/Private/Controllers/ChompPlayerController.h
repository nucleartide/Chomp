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

	UPROPERTY(EditDefaultsOnly, Category = "CustomSettings")
	float TimeForIntendedDirectionToLast = 0.5f;

private:
	float VerticalAxis = 0.0f;
	float HorizontalAxis = 0.0f;
	float TimeOfLastIntendedDirUpdate = 0.0f;

	FGridLocation InitialMoveDirection{0, 1};
	FGridLocation CurrentMoveDirection{0, 1};
	FGridLocation IntendedMoveDirection;

	/**
	 * A flag to denote whether TargetTile is currently set.
	 *
	 * This is necessary because FGridLocation is a struct, and thus there is no "null" value.
	 */
	bool IsTargetTileSet = false;
	FGridLocation TargetTile{0, 0};

public:
	AChompPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void OnMoveHorizontal(float Delta);
	void OnMoveVertical(float Delta);

	UFUNCTION()
	void HandleGameRestarted(EChompGameState OldState, EChompGameState NewState);
};
