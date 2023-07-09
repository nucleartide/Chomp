#include "UI/UIManager.h"
#include "Utils/Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/GameOverWidget.h"
#include "PacmanGameMode.h"
#include "Pawns/PacmanPawn.h"

AUIManager::AUIManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIManager::BeginPlay()
{
	Super::BeginPlay();
	LevelGenerator->OnLevelClearedDelegate.AddUniqueDynamic(this, &AUIManager::HandleDotsCleared);
	PacmanPawn->OnPacmanDiedDelegate.AddUniqueDynamic(this, &AUIManager::HandlePlayerDeath);
}

void AUIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUIManager::HandleDotsCleared()
{
	DEBUG_LOG(TEXT("Dots cleared, showing game over win UI..."))

	auto World = GetWorld();
	check(World);

	auto WidgetInstance = CreateWidget(World, GameOverWinWidgetClass);
	check(WidgetInstance);

	{
		GameOverWidgetInstance = Cast<UGameOverWidget>(WidgetInstance);
		check(GameOverWidgetInstance);
		GameOverWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIManager::HandleRestartGameClicked);
	}

	WidgetInstance->AddToViewport();

	auto ViewportClient = World->GetGameViewport();
	check(ViewportClient);

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
	Controller->SetInputMode(FInputModeGameAndUI());
}

void AUIManager::HandlePlayerDeath()
{
	DEBUG_LOG(TEXT("Player died, showing game over *lose* UI..."))

	auto World = GetWorld();
	check(World);

	auto WidgetInstance = CreateWidget(World, GameOverLoseWidgetClass);
	check(WidgetInstance);

	{
		GameOverWidgetInstance = Cast<UGameOverWidget>(WidgetInstance);
		check(GameOverWidgetInstance);
		GameOverWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIManager::HandleRestartGameClicked);
	}

	WidgetInstance->AddToViewport();

	auto ViewportClient = World->GetGameViewport();
	check(ViewportClient);

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
	Controller->SetInputMode(FInputModeGameAndUI());
}

void AUIManager::HandleRestartGameClicked()
{
	DEBUG_LOG(TEXT("Handling game restarted in UIManager..."))

	// By reaching out to the current game mode, this class becomes coupled to PacmanGameMode.
	// That's okay. This class is intended to be game-specific anyway.
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	// TODO: Does casting to an interface avoid hard references? What is a hard reference anyway, and why is avoiding hard refs important?
	// Something to investigate for later.
	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	PacmanGameMode->SetGameState(PacmanGameState::Playing);

	// Hide the widget.
	GameOverWidgetInstance->RemoveFromParent();
	GameOverWidgetInstance->Destruct();
	GameOverWidgetInstance = nullptr;

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(false);
	Controller->SetInputMode(FInputModeGameOnly());
}
