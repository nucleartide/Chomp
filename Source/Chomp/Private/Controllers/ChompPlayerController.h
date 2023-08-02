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

	// The pawn's initial movement direction.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	FGridLocation InitialMoveDirection{0, 1};

	// Vertical input that is read from keyboard.
	UPROPERTY(VisibleAnywhere)
	float VerticalAxisInput = 0.0f;
	double TimeThatVerticalAxisWasSet = 0.0f;

	// Horizontal input that is read from keyboard.
	UPROPERTY(VisibleAnywhere)
	float HorizontalAxisInput = 0.0f;
	double TimeThatHorizontalAxisWasSet = 0.0f;

	// The next movement that is intended by the player.
	TSharedPtr<FMovementIntention> IntendedMovement;

	// The movement that is currently taking place.
	TSharedPtr<FMovement> CurrentMovement;

	UPROPERTY(VisibleAnywhere)
	FGridLocation DebugCurrentTargetTile;

	// Value for sanity checking.
	UPROPERTY(VisibleAnywhere)
	FVector OldLocation;

public:
	AChompPlayerController();
	
	void HandleGameRestarted(EChompGameState OldState, EChompGameState NewState);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	void OnMoveHorizontal(float Input);

	void OnMoveVertical(float Input);

	TSharedPtr<FMovementIntention> UpdateIntendedMovement() const;

	TSharedPtr<FMovement> UpdateCurrentMovement(const bool InvalidateTargetTile) const;
};
