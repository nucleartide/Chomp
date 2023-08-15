#include "PinkyAiController.h"

FGridLocation APinkyAiController::GetChaseEndGridPosition_Implementation() const
{
	const auto PlayerGridLocation = GetPlayerGridLocation();
	const auto PlayerGridDirection = GetPlayerGridDirection();

	auto PlayerGridAheadLocation = PlayerGridLocation;
	if (PlayerGridDirection.IsNonZero())
	{
		// Get the grid position 400 centimeters ahead (as much as possible) of PlayerGridLocation.
		for (auto i = 0; i < 4; i++)
		{
			const auto LocationToTest = FGridLocation{
				PlayerGridAheadLocation.X + PlayerGridDirection.X,
				PlayerGridAheadLocation.Y + PlayerGridDirection.Y
			};
			if (const auto IsValidLocation = ULevelLoader::GetInstance(Level)->Passable(PlayerGridAheadLocation, LocationToTest))
				PlayerGridAheadLocation = LocationToTest;
			else
				break;
		}
	}

	checkf(ULevelLoader::GetInstance(Level)->Passable(PlayerGridAheadLocation),
	       TEXT("Post condition"));
	return PlayerGridAheadLocation;
}
