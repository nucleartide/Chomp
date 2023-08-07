#include "InkyAiController.h"

#include "ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/GhostPawn.h"
#include "Utils/SafeGet.h"

void AInkyAiController::Initialize(AGhostPawn* BlinkyPawn)
{
	BlinkyPawnRef = BlinkyPawn;
}

FMaybeGridLocation AInkyAiController::GetChaseStartGridPosition_Implementation() const
{
	return GetPlayerGridLocation();
}

FMaybeGridLocation AInkyAiController::GetChaseEndGridPosition_Implementation() const
{
	// Get Blinky's grid position.
	const auto BlinkyGridLocation = BlinkyPawnRef->GetGridLocation();

	// Get the player's grid position.
	const auto [IsValid, PlayerGridLocation] = GetPlayerGridLocation();
	if (!IsValid) return FMaybeGridLocation::Invalid();

	// Get the player's grid direction.
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto ChompPlayerController = Cast<AChompPlayerController>(PlayerController);
	check(ChompPlayerController);
	const auto PlayerGridDirection = ChompPlayerController->GetCurrentMovement();

	// Get the grid position 2 meters ahead (as much as possible) of PlayerGridLocation.
	auto PlayerGridAheadLocation = PlayerGridLocation;
	if (PlayerGridDirection.IsNonZero())
	{
		for (auto i = 0; i < 2; i++)
		{
			const auto LocationToTest = FGridLocation{
				PlayerGridAheadLocation.X + PlayerGridDirection.X,
				PlayerGridAheadLocation.Y + PlayerGridDirection.Y};
			if (const auto IsValidLocation = ULevelLoader::GetInstance(Level)->IsValid(LocationToTest))
				PlayerGridAheadLocation = LocationToTest;
		}
	}

	// Get the results of B - A.
	// Name this vector C.

	// Then, double the vector above.
	// Name this vector D.
	// ...

	// While the position is invalid and greater than the magnitude of C,
	while (false)
	{
		// Decrement the magnitude of vector D by 1.
		// ...
	}

	// Once you are done decrementing, the resulting grid position is your final result.
	// ...

	// End position.
	// ===

	// Also, visualize these start and end positions with debug spheres in the parent AGhostAIController.
	// ...

	return Super::GetChaseEndGridPosition_Implementation();
}

FMaybeGridLocation AInkyAiController::GetPlayerGridLocation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AChompPawn>();
	if (!PlayerPawn)
		return FMaybeGridLocation::Invalid();
	return FMaybeGridLocation::Valid(PlayerPawn->GetGridLocation());
}