#include "ClydeAiController.h"

#include "Pawns/GhostPawn.h"
#include "Pawns/MovablePawn.h"
#include "Utils/SafeGet.h"

void AClydeAiController::DecideToUpdateMovementPathInChase_Implementation(FVector NewLocation)
{
	// Get the current world location of Pacman.
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	const auto PlayerWorldLocation = PlayerPawn->GetActorLocation();

	// Get the current world location of the ghost pawn.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto GhostWorldLocation = GhostPawn->GetActorLocation();

	// Compute the distance between the 2 world locations above.
	const auto Distance = (PlayerWorldLocation - GhostWorldLocation).Length();

	// Compute ShouldChasePacman:
	// * True if Clyde's distance to Pacman is < CentimetersAwayFromPacman
	// * False otherwise.
	ShouldChasePacman = Distance < CentimetersAwayFromPacman;

	if (
		// We should be chasing Pacman, and
		ShouldChasePacman &&
		(
			// We've just crossed the CentimetersAwayFromPacman threshold, or
			ShouldChasePacman != WasChasingPacman ||
			// We've traversed one node along the path (Pacman may have shifted positions since computing the original path), or
			MovementPath.GetWorldLocationPath().Num() >= 2 && MovementPath.DidComplete(NewLocation, 1) ||
			// We've completed a one-node path,
			MovementPath.GetWorldLocationPath().Num() == 1 && MovementPath.WasCompleted(NewLocation)
		)
	)
	{
		MovementPath = UpdateMovementPathWhenInChase();
	}
	else if (
		// We should not be chasing Pacman, and
		!ShouldChasePacman &&
		(
			// We've just crossed the CentimetersAwayFromPacman threshold, or
			ShouldChasePacman != WasChasingPacman ||
			// We've completed the path,
			MovementPath.WasCompleted(NewLocation)
		)
	)
	{
		MovementPath = UpdateMovementPathWhenInScatter();
		std::swap(CurrentScatterOrigin, CurrentScatterDestination);
	}

	// Update bookkeeping.
	WasChasingPacman = ShouldChasePacman;
}

FGridLocation AClydeAiController::GetChaseEndGridPosition_Implementation() const
{
	if (ShouldChasePacman)
	{
		const auto PlayerGridLocation = GetPlayerGridLocation();
		return PlayerGridLocation;
	}

	return CurrentScatterDestination;
}
