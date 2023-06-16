#include "TestActor.h"
#include "Debug.h"
#include "PacmanGameMode.h"

ATestActor::ATestActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

/*
	// Fetch the game mode.
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	// Cast to PacmanGameMode.
	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	// Log the value of the GameState.
	auto GameState = PacmanGameMode->GameState;
	DEBUG_LOG(TEXT("Current GameState enum: %d"), GameState);
*/

	{
		Level.GetDefaultObject()->LoadLevel();
		DEBUG_LOG(TEXT("Level width: %d"), Level.GetDefaultObject()->GetLevelWidth());
		DEBUG_LOG(TEXT("Level height: %d"), Level.GetDefaultObject()->GetLevelHeight());
	}

	{
		DEBUG_LOG(TEXT("Level width: %d"), Level.GetDefaultObject()->GetLevelWidth());
		DEBUG_LOG(TEXT("Level height: %d"), Level.GetDefaultObject()->GetLevelHeight());
	}
}

void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
