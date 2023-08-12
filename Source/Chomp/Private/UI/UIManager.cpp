#include "UI/UIManager.h"
#include "Utils/Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/GameOverWidget.h"
#include "ChompGameState.h"

AUIManager::AUIManager(): AActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIManager::BeginPlay()
{
	Super::BeginPlay();
	const auto GameState = GetWorld()->GetGameState<AChompGameState>();
	GameState->OnDotsClearedDelegate.AddUniqueDynamic(this, &AUIManager::HandleDotsCleared);
	GameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AUIManager::HandlePlayerDeath);
}

void AUIManager::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUIManager::HandleDotsCleared()
{
	DEBUG_LOG(TEXT("Dots cleared, showing game over win UI..."))

	GameOverWidgetInstance = Cast<UGameOverWidget>(CreateWidget(GetWorld(), GameOverWinWidgetClass));
	GameOverWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIManager::HandleRestartGameClicked);
	GameOverWidgetInstance->AddToViewport();

	const auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
	Controller->SetInputMode(FInputModeGameAndUI());
}

void AUIManager::HandlePlayerDeath(const EChompGameStateEnum OldState, const EChompGameStateEnum NewState)
{
	const auto DidLose = OldState != NewState && NewState == EChompGameStateEnum::GameOverLose;
	if (!DidLose)
		return;

	DEBUG_LOG(TEXT("Player died, showing game over *lose* UI..."))

	GameOverWidgetInstance = Cast<UGameOverWidget>(CreateWidget(GetWorld(), GameOverLoseWidgetClass));
	GameOverWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIManager::HandleRestartGameClicked);
	GameOverWidgetInstance->AddToViewport();

	const auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
	Controller->SetInputMode(FInputModeGameAndUI());
}

void AUIManager::HandleRestartGameClicked()
{
	// Perform game state transition.
	DEBUG_LOG(TEXT("Handling game restarted in UIManager..."))
	GetWorld()->GetGameState<AChompGameState>()->StartGame();

	// Hide the widget.
	GameOverWidgetInstance->RemoveFromParent();
	GameOverWidgetInstance->Destruct();
	GameOverWidgetInstance = nullptr;

	// Reset mouse cursor state.
	const auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(false);
	Controller->SetInputMode(FInputModeGameOnly());
}
