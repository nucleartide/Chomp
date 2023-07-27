#include "Controllers/ChompPlayerController.h"
#include "Pawns/MovablePawn.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"
#include "Engine/World.h"
#include "Utils/SafeGet.h"
#include "Pawns/Movement/Movement.h"
#include "Pawns/Movement/MovementIntention.h"

AChompPlayerController::AChompPlayerController(): APlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Disable this. We want to manage the active camera ourselves.
	bAutoManageActiveCameraTarget = false;
}

void AChompPlayerController::OnMoveVertical(const float Input)
{
	VerticalAxisInput = Input;
}

void AChompPlayerController::OnMoveHorizontal(const float Input)
{
	HorizontalAxisInput = Input;
}

TSharedPtr<FMovementIntention> AChompPlayerController::UpdateIntendedMovement() const
{
	const auto WorldInstance = FSafeGet::World(this);

	if (VerticalAxisInput != 0.0f || HorizontalAxisInput != 0.0f)
		return MakeShared<FMovementIntention>(VerticalAxisInput, HorizontalAxisInput, WorldInstance);

	if (IntendedMovement->SinceLastUpdate(TimeForIntendedDirectionToLast, WorldInstance))
		return MakeShared<FMovementIntention>(0.0f, 0.0f, WorldInstance);

	return IntendedMovement;
}

TSharedPtr<FMovement> AChompPlayerController::UpdateCurrentMovement(const bool InvalidateTargetTile) const
{
	const auto NewMovement = InvalidateTargetTile
		                         ? MakeShared<FMovement>(CurrentMovement->Direction,
		                                                 FMaybeGridLocation{false, FGridLocation{0, 0}})
		                         : CurrentMovement;

	if (NewMovement->HasValidTargetTile())
		return NewMovement;

	if (!IntendedMovement->IsDifferentFrom(NewMovement))
		return NewMovement;

	if (const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
		Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement->Direction))
	{
		const auto CurrentGridLocation = Pawn->GetGridLocation();
		const auto NextGridLocation = CurrentGridLocation + IntendedMovement->Direction;
		return MakeShared<FMovement>(IntendedMovement->Direction, FMaybeGridLocation{true, NextGridLocation});
	}

	return NewMovement;
}

void AChompPlayerController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->GetEnum() != EChompGameState::Playing)
		return;

	const auto MovablePawn = GetPawn<AMovablePawn>();
	if (!MovablePawn)
		return;

	const auto [NewLoc, NewRot, InvalidateTargetTile] = MovablePawn->MoveInDirection(
		CurrentMovement,
		IntendedMovement,
		DeltaTime);
	MovablePawn->SetActorLocationAndRotation(NewLoc, NewRot);
	
	IntendedMovement = UpdateIntendedMovement();
	CurrentMovement = UpdateCurrentMovement(InvalidateTargetTile);
}

void AChompPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Bind input axes.
	InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
	InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);

	// Bind restart handler.
	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
	GameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AChompPlayerController::HandleGameRestarted);
}

void AChompPlayerController::HandleGameRestarted(EChompGameState OldState, EChompGameState NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGameState::Playing)
	{
		const auto World = FSafeGet::World(this);
		IntendedMovement = MakeShared<FMovementIntention>(InitialMoveDirection, World);
	}
}
