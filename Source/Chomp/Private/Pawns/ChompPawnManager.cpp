#include "Pawns/ChompPawnManager.h"
#include "ChompGameMode.h"
#include "Pawns/ChompPawn.h"

AChompPawnManager::AChompPawnManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChompPawnManager::BeginPlay()
{
	Super::BeginPlay();

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto ChompGameMode = Cast<AChompGameMode>(GameMode);
	check(ChompGameMode);

	ChompGameMode->OnGameRestartedDelegate.AddUniqueDynamic(this, &AChompPawnManager::HandleGameRestarted);
	ChompPawn->OnPacmanDiedDelegate.AddUniqueDynamic(this, &AChompPawnManager::HandlePacmanDied);

	ChompPawn->SetActorLocation(GetActorLocation());
}

void AChompPawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChompPawnManager::HandleGameRestarted()
{
	ChompPawn->SetActorLocation(GetActorLocation());
}

void AChompPawnManager::HandlePacmanDied()
{
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto ChompGameMode = Cast<AChompGameMode>(GameMode);
	check(ChompGameMode);

	ChompGameMode->SetGameState(PacmanGameState::GameOverLose);
}
