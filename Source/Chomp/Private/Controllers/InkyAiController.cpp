#include "InkyAiController.h"

#include "ChompPlayerController.h"
#include "Pawns/GhostPawn.h"

void AInkyAiController::Initialize(AGhostPawn* BlinkyPawn)
{
	BlinkyPawnRef = BlinkyPawn;
}

FGridLocation AInkyAiController::GetChaseEndGridPosition_Implementation() const
{
	// Get the grid position under Blinky.
	const auto BlinkyGridLocation = BlinkyPawnRef->GetGridLocation();

	// Get the player's info.
	const auto PlayerGridLocation = GetPlayerGridLocation();
	const auto PlayerGridDirection = GetPlayerGridDirection();

	// Get the grid position 2 meters ahead (as much as possible) of PlayerGridLocation.
	auto PlayerGridAheadLocation = PlayerGridLocation;
	if (PlayerGridDirection.IsNonZero())
	{
		for (auto i = 0; i < 2; i++)
		{
			const auto LocationToTest = FGridLocation{
				PlayerGridAheadLocation.X + PlayerGridDirection.X,
				PlayerGridAheadLocation.Y + PlayerGridDirection.Y
			};
			if (const auto IsValidLocation =
				ULevelLoader::GetInstance(Level)->Passable(PlayerGridAheadLocation, LocationToTest))
			{
				PlayerGridAheadLocation = LocationToTest;
			}
			else
			{
				break;
			}
		}
	}

	// Get the the difference vector (world space) of the previous 2 results.
	// Name this vector C.
	const auto C = (PlayerGridAheadLocation.ToFVector() - BlinkyGridLocation.ToFVector()) * 100.0;
	double CMagnitude;
	FVector CDirection;
	C.ToDirectionAndLength(CDirection, CMagnitude);

	// Then, double the vector above.
	// Name this vector D.
	auto DMagnitude = 2.0 * CMagnitude;
	auto D = DMagnitude * CDirection;

	// Inch backward until we have a valid grid position.
	// If we can't find *any* valid grid position, then default to the PlayerGridAheadLocation.
	auto PendingEndWorldPos = BlinkyPawnRef->GetActorLocation() + D;
	while (
		DMagnitude > CMagnitude &&
		!ULevelLoader::GetInstance(Level)->Passable(PendingEndWorldPos)
	)
	{
		// Decrement the magnitude of vector D by 100 cm.
		DMagnitude -= 100.0;

		// Recompute D.
		D = DMagnitude > CMagnitude ? DMagnitude * CDirection : CMagnitude * CDirection;

		// Recompute PendingEndWorldPos.
		PendingEndWorldPos = BlinkyPawnRef->GetActorLocation() + D;
	}

	// This should always be true.
	checkf(
		ULevelLoader::GetInstance(Level)->Passable(PendingEndWorldPos),
		TEXT("PendingEndWorldPos can always fall back to the player's direct position.")
	);

	// Once you are done decrementing, the resulting grid position is your final result.
	const auto ResultGridPosition = ULevelLoader::GetInstance(Level)->WorldToGrid(FVector2D(PendingEndWorldPos));
	return ResultGridPosition;
}
