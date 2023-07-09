#include "Pawns/PacmanPawnManager.h"
#include "PacmanGameMode.h"
#include "Pawns/PacmanPawn.h"

APacmanPawnManager::APacmanPawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APacmanPawnManager::BeginPlay()
{
	Super::BeginPlay();

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	PacmanGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &APacmanPawnManager::HandleGameRestarted);
	PacmanPawn->OnPacmanDiedDelegate.AddUniqueDynamic(this, &APacmanPawnManager::HandlePacmanDied);

	PacmanPawn->SetActorLocation(GetActorLocation());
}

void APacmanPawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APacmanPawnManager::HandleGameRestarted()
{
	PacmanPawn->SetActorLocation(GetActorLocation());
}

void APacmanPawnManager::HandlePacmanDied()
{
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	PacmanGameMode->SetGameState(PacmanGameState::GameOverLose);
}
