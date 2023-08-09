#include "InkyAiPossessor.h"

#include "InkyAiController.h"
#include "Utils/SafeGet.h"
#include "Pawns/GhostPawn.h"

void AInkyAiPossessor::BeginPlay()
{
	Super::BeginPlay();

	// Preconditions.
	check(InkyAiController);
	check(BlinkyPawn);
	check(InkyPawn);

	// Spawn an InkyAiController.
	const auto World = FSafeGet::World(this);
	const auto InkyController = World->SpawnActor<AInkyAiController>(InkyAiController);
	check(InkyController);

	// Remember to initialize.
	InkyController->Initialize(BlinkyPawn);

	// Possess InkyPawn using the spawned InkyAiController.
	InkyController->Possess(InkyPawn);
}
