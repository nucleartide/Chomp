#include "PacmanPawnManager.h"
#include "PacmanGameMode.h"
#include "PacmanPawn.h"

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
}

void APacmanPawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APacmanPawnManager::HandleGameRestarted()
{
	PacmanPawn->SetActorLocation(GetActorLocation());
}
