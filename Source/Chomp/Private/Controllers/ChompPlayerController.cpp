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
	const auto World = FSafeGet::World(this);
	if (FMath::IsNearlyEqual(FMath::Abs(Input), 1.0))
		TimeThatVerticalAxisWasSet = World->GetRealTimeSeconds();
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
	const auto World = FSafeGet::World(this);
	if (FMath::IsNearlyEqual(FMath::Abs(Input), 1.0))
		TimeThatHorizontalAxisWasSet = World->GetRealTimeSeconds();
}

TSharedPtr<FMovementIntention> AChompPlayerController::UpdateIntendedMovement() const
{
	const auto WorldInstance = FSafeGet::World(this);
	
	if (!IntendedMovement.IsValid())
		return MakeShared<FMovementIntention>(0.0, 0.0, WorldInstance);

	if (VerticalAxisInput != 0.0f || HorizontalAxisInput != 0.0f)
	{
		// Prioritize the latest pressed key.
		if (TimeThatVerticalAxisWasSet > TimeThatHorizontalAxisWasSet)
			return MakeShared<FMovementIntention>(VerticalAxisInput, 0.0, WorldInstance);

		return MakeShared<FMovementIntention>(0.0, HorizontalAxisInput, WorldInstance);
	}

	return IntendedMovement;
}

TSharedPtr<FMovement> NewMovement(
	const AMovablePawn* Pawn,
	const FGridLocation& Direction,
	const ULevelLoader* LevelInstance)
{
	const auto CurrentGridLocation = Pawn->GetGridLocation();
	const auto NextGridLocation = (CurrentGridLocation + Direction).Modulo(LevelInstance);
	return MakeShared<FMovement>(Direction, FMaybeGridLocation{true, NextGridLocation});
}

void CheckThatPawnIsRightOnGrid(const AMovablePawn* Pawn)
{
	const auto Location = Pawn->GetActorLocation();
	check(FMath::IsNearlyEqual(MathHelpers::NotStupidFmod(Location.X, 100.0), 0.0));
	check(FMath::IsNearlyEqual(MathHelpers::NotStupidFmod(Location.Y, 100.0), 0.0));
	check(FMath::IsNearlyEqual(Location.Z, 0.0));
}

TSharedPtr<FMovement> AChompPlayerController::UpdateCurrentMovement(const bool InvalidateTargetTile = false) const
{
	DEBUG_LOG(TEXT("updating current movement"));
	
	// Grab reference to pawn.
	// Pawn may be dead at this point, so we should return early if needed.
	const auto Pawn = GetPawn<AMovablePawn>();
	if (!Pawn)
		return CurrentMovement;

	// Case where current movement hasn't been set yet.
	if (!CurrentMovement.IsValid())
	{
		CheckThatPawnIsRightOnGrid(Pawn);

		if (Pawn->CanTravelInDirection(Pawn->GetActorLocation(), InitialMoveDirection))
		{
			DEBUG_LOG(TEXT("first"));
			return NewMovement(Pawn, InitialMoveDirection, ULevelLoader::GetInstance(Level));
		}

		DEBUG_LOG(TEXT("second"));
		return MakeShared<FMovement>(InitialMoveDirection, FMaybeGridLocation{false, FGridLocation{0, 0}});
	}

	// Case where we are doing a 180.
	if (IntendedMovement->Direction.IsNonZero() &&
		IntendedMovement->Direction.IsOppositeDirection(CurrentMovement->Direction) &&
		Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement->Direction))
	{
		DEBUG_LOG(TEXT("third"));
		return NewMovement(Pawn, IntendedMovement->Direction, ULevelLoader::GetInstance(Level));
	}

	// Case where we can compute a new target.
	if (InvalidateTargetTile || !CurrentMovement->HasValidTargetTile())
	{
		// Case where we can move in intended movement direction.
		if (IntendedMovement->Direction.IsNonZero() &&
			IntendedMovement->Direction != CurrentMovement->Direction &&
			Pawn->CanTravelInDirection(Pawn->GetActorLocation(), IntendedMovement->Direction))
		{
			DEBUG_LOG(TEXT("fourth"));
			CheckThatPawnIsRightOnGrid(Pawn);
			return NewMovement(Pawn, IntendedMovement->Direction, ULevelLoader::GetInstance(Level));
		}

		// Case where we can continue moving in current direction.
		if (Pawn->CanTravelInDirection(Pawn->GetActorLocation(), CurrentMovement->Direction))
		{
			DEBUG_LOG(TEXT("fifth"));
			return NewMovement(Pawn, CurrentMovement->Direction, ULevelLoader::GetInstance(Level));
		}

		// Case where we can't move anywhere.
		DEBUG_LOG(TEXT("sixth"));
		CheckThatPawnIsRightOnGrid(Pawn);
		return MakeShared<FMovement>(CurrentMovement->Direction, FMaybeGridLocation{false, FGridLocation{0, 0}});
	}

	// Default, return the current movement.
	DEBUG_LOG(TEXT("default"));
	return CurrentMovement;
}

void AChompPlayerController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->GetEnum() != EChompGameState::Playing)
		return;

	const auto MovablePawn = GetPawn<AMovablePawn>();
	if (!IsValid(MovablePawn))
		return;

	DEBUG_LOG(TEXT("beginning of tick"));

	IntendedMovement = UpdateIntendedMovement();
		check(IntendedMovement->Direction.IsCardinalOrZero());

	auto ShouldInvalidateTargetTile = false;

	if (CurrentMovement.IsValid() && CurrentMovement->HasValidTargetTile())
	{
		const auto [NewLoc, NewRot, InvalidateTargetTile, CanTravelInIntendedDir] = MovablePawn->MoveInDirection(
			CurrentMovement,
			IntendedMovement,
			DeltaTime);
		MovablePawn->SetActorLocationAndRotation(NewLoc, NewRot);
		ShouldInvalidateTargetTile = InvalidateTargetTile;
	}

	CurrentMovement = UpdateCurrentMovement(ShouldInvalidateTargetTile);

	if (CurrentMovement.IsValid())
	{
		DebugCurrentTargetTile = CurrentMovement->TargetTile.GridLocation;
	}
}

void AChompPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Bind input axes.
	InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
	InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);
}
