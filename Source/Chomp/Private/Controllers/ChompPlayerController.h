#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AStar/GridLocation.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/MovablePawn.h"

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
	FComputeTargetTileResult Target;

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

	void UpdateIntendedMoveDirection();
	static void UpdateCurrentMoveDirectionAndTarget(
		FGridLocation &CurrentMoveDirection,
		FComputeTargetTileResult &Target,
		FGridLocation IntendedMoveDirection,
		UWorld *World,
		AMovablePawn *MovablePawn,
		ULevelLoader *LevelInstance,
		float DeltaTime);
};
