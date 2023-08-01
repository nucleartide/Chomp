#include "Controllers/ChompPlayerController.h"
#include "Pawns/MovablePawn.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"
#include "Engine/World.h"
#include "Utils/SafeGet.h"
#include "Pawns/Movement/Movement.h"
#include "Pawns/Movement/MovementIntention.h"
#include "Utils/MathHelpers.h"

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

void AChompPlayerController::HandleGameRestarted(EChompGameState OldState, EChompGameState NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGameState::Playing)
	{
		CurrentMovement.Reset();
		IntendedMovement.Reset();
	}
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

	if (!IntendedMovement.IsValid())
		return MakeShared<FMovementIntention>(0.0f, 0.0f, WorldInstance);

	if (IntendedMovement->Direction.IsNonZero() &&
		IntendedMovement->HasElapsedSinceLastUpdate(TimeForIntendedDirectionToLast, WorldInstance))
		return MakeShared<FMovementIntention>(0.0f, 0.0f, WorldInstance);

	return IntendedMovement;
}

TSharedPtr<FMovement> AChompPlayerController::UpdateCurrentMovement(const bool InvalidateTargetTile = false) const
{
	if (!CurrentMovement.IsValid())
		return ComputeMovementWithTargetTile(InitialMoveDirection);

	if (const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
		IntendedMovement->Direction.IsNonZero() &&
		IntendedMovement->Direction != CurrentMovement->Direction &&
		Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement->Direction))
	{
		const auto CurrentGridLocation = Pawn->GetGridLocation();
		const auto NextGridLocation = CurrentGridLocation + IntendedMovement->Direction;
		return MakeShared<FMovement>(IntendedMovement->Direction, FMaybeGridLocation{true, NextGridLocation});
	}

	if (!CurrentMovement->HasValidTargetTile() || InvalidateTargetTile)
		return ComputeMovementWithTargetTile(CurrentMovement->Direction);

	return CurrentMovement;
}

TSharedPtr<FMovement> AChompPlayerController::ComputeMovementWithTargetTile(FGridLocation Direction) const
{
	if (const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
		Pawn->CanTravelInDirection(Pawn->GetActorLocation(), Direction))
	{
		const auto CurrentGridLocation = Pawn->GetGridLocation();
		const auto NextGridLocation = CurrentGridLocation + Direction;
		return MakeShared<FMovement>(Direction, FMaybeGridLocation{true, NextGridLocation});
	}

	return MakeShared<FMovement>(Direction, FMaybeGridLocation{false, FGridLocation{0, 0}});
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

	IntendedMovement = UpdateIntendedMovement();

		CurrentMovement = UpdateCurrentMovement(ShouldInvalidateTargetTile);
#if false
	{
		const auto OldCurrentMovement = CurrentMovement;
		CurrentMovement = UpdateCurrentMovement(ShouldInvalidateTargetTile);
		if (OldCurrentMovement.IsValid() && OldCurrentMovement->HasValidTargetTile() && OldCurrentMovement->Direction.IsTurningCorner(CurrentMovement->Direction))
		{
			const auto CurrentLocation = MovablePawn->GetActorLocation();
			const auto TargetWorld =
				ULevelLoader::GetInstance(Level)->GridToWorld(OldCurrentMovement->TargetTile.GridLocation);
			MovablePawn->SetActorLocation(FVector{TargetWorld.X, TargetWorld.Y, 0.0});
		}
	}
#endif

	if (CurrentMovement->HasValidTargetTile())
	{
		const auto [NewLoc, NewRot, InvalidateTargetTile] = MovablePawn->MoveInDirection(
			CurrentMovement,
			IntendedMovement,
			DeltaTime);
		MovablePawn->SetActorLocationAndRotation(NewLoc, NewRot);
		ShouldInvalidateTargetTile = InvalidateTargetTile;
		

	}


}

void AChompPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Bind input axes.
	InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
	InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);
}
