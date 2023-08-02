#pragma once

#include "CoreMinimal.h"
#include "ChompGameState.h"
#include "GameFramework/PlayerController.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/Movement/Movement.h"
#include "Pawns/Movement/MovementIntention.h"

#include "ChompPlayerController.generated.h"

UCLASS()
class AChompPlayerController : public APlayerController
{
	GENERATED_BODY()

	// Reference to level.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	// The pawn's initial movement direction.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation InitialMoveDirection{0, 1};

	// Vertical input that is read from keyboard.
	UPROPERTY(VisibleAnywhere)
	float VerticalAxisInput = 0.0f;

	// Time when the vertical input was last read.
	UPROPERTY(VisibleAnywhere)
	double TimeThatVerticalAxisWasSet = 0.0f;

	// Horizontal input that is read from keyboard.
	UPROPERTY(VisibleAnywhere)
	float HorizontalAxisInput = 0.0f;
	
	// Time when the horizontal input was last read.
	UPROPERTY(VisibleAnywhere)
	double TimeThatHorizontalAxisWasSet = 0.0f;
	
	UPROPERTY(VisibleAnywhere)
	double TimeBeforeClearingIntendedMovement = 0.25f;

	UPROPERTY(VisibleAnywhere)
	FMovementIntention IntendedMovement;

	UPROPERTY(VisibleAnywhere)
	FMovement CurrentMovement;

public:
	// Constructor to enable ticks.
	AChompPlayerController();

	// Handle game restarts.
	void HandleGameRestarted(EChompGameState OldState, EChompGameState NewState);

protected:
	// Needed to attach some handlers.
	virtual void BeginPlay() override;

	// Needed to run per-frame logic.
	virtual void Tick(float DeltaTime) override;

private:
	// Capture horizontal input.
	void OnMoveHorizontal(float Input);

	// Capture vertical input.
	void OnMoveVertical(float Input);

	FMovementIntention UpdateIntendedMovement() const;

	FMovement UpdateCurrentMovement(const bool InvalidateTargetTile) const;

	void ResetMovement();
};
