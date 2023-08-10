#include "Controllers/ChompPlayerController.h"
#include "Pawns/MovablePawn.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"
#include "Engine/World.h"
#include "Pawns/Movement/MoveInDirectionResult.h"
#include "Utils/SafeGet.h"
#include "Pawns/Movement/Movement.h"
#include "Pawns/Movement/MovementIntention.h"
#include "Utils/MathHelpers.h"

void CheckThatPawnIsRightOnGrid(const AMovablePawn* Pawn)
{
	const auto Location = Pawn->GetActorLocation();
	check(FMath::IsNearlyEqual(FMathHelpers::NegativeFriendlyFmod(Location.X, 100.0), 0.0));
	check(FMath::IsNearlyEqual(FMathHelpers::NegativeFriendlyFmod(Location.Y, 100.0), 0.0));
	check(FMath::IsNearlyEqual(Location.Z, 0.0));
}

FMovement NewMovement(
	const AMovablePawn* Pawn,
	const FGridLocation& Direction,
	const ULevelLoader* LevelInstance)
{
	const auto CurrentGridLocation = Pawn->GetGridLocation();
	const auto NextGridLocation = (CurrentGridLocation + Direction).Modulo(LevelInstance);
	return FMovement(Direction, FMaybeGridLocation{true, NextGridLocation});
}

AChompPlayerController::AChompPlayerController(): APlayerController()
{
	// Enable Tick() function.
	PrimaryActorTick.bCanEverTick = true;

	// Disable this. We want to manage the active camera ourselves.
	bAutoManageActiveCameraTarget = false;
}

void AChompPlayerController::OnMoveVertical(const float Input)
{
	VerticalAxisInput = Input;
	const auto World = FSafeGet::World(this);
	if (FMath::IsNearlyEqual(FMath::Abs(Input), 1.0))
		TimeThatVerticalAxisWasSet = World->GetRealTimeSeconds();
}

void AChompPlayerController::HandleGameRestarted(EChompGameStateEnum OldState, EChompGameStateEnum NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGameStateEnum::Playing)
		ResetMovement();
}

FGridLocation AChompPlayerController::GetCurrentMovement() const
{
	return CurrentMovement.GetDirection();
}

void AChompPlayerController::OnMoveHorizontal(const float Input)
{
	HorizontalAxisInput = Input;
	const auto World = FSafeGet::World(this);
	if (FMath::IsNearlyEqual(FMath::Abs(Input), 1.0))
		TimeThatHorizontalAxisWasSet = World->GetRealTimeSeconds();
}

FMovementIntention AChompPlayerController::UpdateIntendedMovement() const
{
	const auto WorldInstance = FSafeGet::World(this);

	if (VerticalAxisInput != 0.0f || HorizontalAxisInput != 0.0f)
	{
		// Prioritize the latest pressed key.
		if (TimeThatVerticalAxisWasSet > TimeThatHorizontalAxisWasSet)
			return FMovementIntention(VerticalAxisInput, 0.0, WorldInstance);

		return FMovementIntention(0.0, HorizontalAxisInput, WorldInstance);
	}

	if (IntendedMovement.HasElapsedSinceLastUpdate(TimeBeforeClearingIntendedMovement, WorldInstance))
		return FMovementIntention(0.0, 0.0, WorldInstance);

	return IntendedMovement;
}

FMovement AChompPlayerController::UpdateCurrentMovement(const bool InvalidateTargetTile = false) const
{
	// Grab reference to pawn.
	// Pawn may be dead at this point, so we should return early if needed.
	const auto Pawn = GetPawn<AMovablePawn>();
	if (!Pawn)
		return CurrentMovement;

	// Case where we are doing a 180.
	if (IntendedMovement.GetDirection().IsNonZero() &&
		IntendedMovement.GetDirection().IsOppositeDirection(CurrentMovement.GetDirection()) &&
		Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement.GetDirection()))
	{
		DEBUG_LOG(TEXT("Doing a 180"));
		return NewMovement(Pawn, IntendedMovement.GetDirection(), ULevelLoader::GetInstance(Level));
	}

	// Case where we can compute a new target.
	if (InvalidateTargetTile || !CurrentMovement.HasValidTargetTile())
	{
		// Case where we can move in intended movement direction.
		if (IntendedMovement.GetDirection().IsNonZero() &&
			IntendedMovement.GetDirection() != CurrentMovement.GetDirection() &&
			Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement.GetDirection()))
		{
			DEBUG_LOG(TEXT("Switching to intended movement direction"));
			CheckThatPawnIsRightOnGrid(Pawn);
			return NewMovement(Pawn, IntendedMovement.GetDirection(), ULevelLoader::GetInstance(Level));
		}

		// Case where we can continue moving in current direction.
		if (Pawn->CanTravelInDirection(Pawn->GetActorLocation(), CurrentMovement.GetDirection()))
		{
			DEBUG_LOG(TEXT("Continuing to move in current direction with new target tile"));
			return NewMovement(Pawn, CurrentMovement.GetDirection(), ULevelLoader::GetInstance(Level));
		}

		// Case where we can't move anywhere.
		DEBUG_LOG(TEXT("Can't move anywhere"));
		CheckThatPawnIsRightOnGrid(Pawn);
		return FMovement(CurrentMovement.GetDirection(), FMaybeGridLocation{false, FGridLocation{0, 0}});
	}

	// Default, return the current movement.
	DEBUG_LOG(TEXT("Player: Continuing to move in current direction with old target tile"));
	return CurrentMovement;
}

void AChompPlayerController::ResetMovement()
{
	// Reset intended movement.
	IntendedMovement.Reset();

	// Reset current movement.
	if (const auto Pawn = GetPawn<AMovablePawn>())
	{
		CheckThatPawnIsRightOnGrid(Pawn);
		if (Pawn->CanTravelInDirection(Pawn->GetActorLocation(), InitialMoveDirection))
			CurrentMovement = NewMovement(Pawn, InitialMoveDirection, ULevelLoader::GetInstance(Level));
		else
			CurrentMovement = FMovement(InitialMoveDirection, FMaybeGridLocation{false, FGridLocation{0, 0}});
	}
}

void AChompPlayerController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->GetEnum() != EChompGameStateEnum::Playing)
		return;

	const auto MovablePawn = GetPawn<AMovablePawn>();
	if (!IsValid(MovablePawn))
		return;

	// Update intended movement.
	IntendedMovement = UpdateIntendedMovement();
	check(IntendedMovement.GetDirection().IsCardinalOrZero());

	// Update location and rotation.
	auto ShouldInvalidateTargetTile = false;
	if (CurrentMovement.HasValidTargetTile())
	{
		const auto [NewLoc, NewRot, InvalidateTargetTile] = MovablePawn->MoveInDirection(
			CurrentMovement,
			IntendedMovement,
			DeltaTime
		);
		MovablePawn->SetActorLocationAndRotation(NewLoc, NewRot);
		ShouldInvalidateTargetTile = InvalidateTargetTile;
	}

	// Update current movement.
	CurrentMovement = UpdateCurrentMovement(ShouldInvalidateTargetTile);
}

void AChompPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
	InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);
	
	ResetMovement();
}
