#include "PlayerManager.h"
#include "PacmanGameMode.h"
#include "PacmanPawn.h"

APlayerManager::APlayerManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerManager::BeginPlay()
{
	Super::BeginPlay();

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	PacmanGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &APlayerManager::HandleGameRestarted);
}

void APlayerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerManager::HandleGameRestarted()
{
	PacmanPawn->SetActorLocation(GetActorLocation());
}
