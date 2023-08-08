#include "InkyAiController.h"

#include "ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/GhostPawn.h"
#include "Utils/SafeGet.h"

void AInkyAiController::Initialize(AGhostPawn* BlinkyPawn)
{
	BlinkyPawnRef = BlinkyPawn;
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
				PlayerGridAheadLocation.Y + PlayerGridDirection.Y
			};

			if (const auto IsValidLocation = ULevelLoader::GetInstance(Level)->IsValid(LocationToTest))
				PlayerGridAheadLocation = LocationToTest;
			else
				break;
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

	// While the position is invalid and greater than the magnitude of C,
	auto PendingEndWorldPos = BlinkyPawnRef->GetActorLocation() + D;
	while (DMagnitude > CMagnitude && !ULevelLoader::GetInstance(Level)->IsValid(PendingEndWorldPos))
	{
		// Decrement the magnitude of vector D by 100 cm.
		DMagnitude -= 100.0;

		// Recompute D.
		D = DMagnitude > CMagnitude ? DMagnitude * CDirection : CMagnitude * CDirection;

		// Recompute PendingEndWorldPos.
		PendingEndWorldPos = BlinkyPawnRef->GetActorLocation() + D;
	}

	// This should always be true.
	check(ULevelLoader::GetInstance(Level)->IsValid(PendingEndWorldPos));

	// Once you are done decrementing, the resulting grid position is your final result.
	const auto ResultGridPosition = ULevelLoader::GetInstance(Level)->WorldToGrid(FVector2D(PendingEndWorldPos));
	return FMaybeGridLocation{true, ResultGridPosition};

	// TODO, after dinner:
	// [x] debug the implementation above
	// [ ] visualize these start and end positions with debug spheres in the parent AGhostAIController.
	// [ ] add implementations for the 2 other ghosts
	// [ ] test the 2 other implementations
}

FMaybeGridLocation AInkyAiController::GetPlayerGridLocation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AChompPawn>();
	if (!PlayerPawn)
		return FMaybeGridLocation::Invalid();
	return FMaybeGridLocation::Valid(PlayerPawn->GetGridLocation());
}
