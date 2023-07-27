#pragma once

#include "CoreMinimal.h"
#include "ChompGameState.h"
#include "GameFramework/PlayerController.h"
#include "LevelGenerator/LevelLoader.h"

#include "ChompPlayerController.generated.h"

struct FMovement;
struct FMovementIntention;
struct FGridLocation;

UCLASS()
class AChompPlayerController : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	double TimeForIntendedDirectionToLast = 0.5;

	// The pawn's initial movement direction.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation InitialMoveDirection{0, 1};

	// Vertical input that is read from keyboard.
	float VerticalAxisInput = 0.0f;

	// Horizontal input that is read from keyboard.
	float HorizontalAxisInput = 0.0f;

	// The next movement that is intended by the player.
	TSharedPtr<FMovementIntention> IntendedMovement;

	// The movement that is currently taking place.
	TSharedPtr<FMovement> CurrentMovement;

public:
	AChompPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	void OnMoveHorizontal(float Input);

	void OnMoveVertical(float Input);

	UFUNCTION()
	void HandleGameRestarted(EChompGameState OldState, EChompGameState NewState);

	TSharedPtr<FMovementIntention> UpdateIntendedMovement() const;

	TSharedPtr<FMovement> UpdateCurrentMovement(const bool InvalidateTargetTile) const;
};
