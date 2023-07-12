#include "UI/UIManager.h"
#include "Utils/Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/GameOverWidget.h"
#include "ChompGameMode.h"
#include "Pawns/ChompPawn.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AUIManager::AUIManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIManager::BeginPlay()
{
	Super::BeginPlay();
	LevelGenerator->OnLevelClearedDelegate.AddUniqueDynamic(this, &AUIManager::HandleDotsCleared);
	ChompPawn->OnPacmanDiedDelegate.AddUniqueDynamic(this, &AUIManager::HandlePlayerDeath);
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

// TODO: fix score updating
// TODO: unit test state updates
	WidgetInstance->AddToViewport();

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

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
	Controller->SetInputMode(FInputModeGameAndUI());
}

void AUIManager::HandleRestartGameClicked()
{
	DEBUG_LOG(TEXT("Handling game restarted in UIManager..."))

	// By reaching out to the current game mode, this class becomes coupled to ChompGameMode.
	// That's okay. This class is intended to be game-specific anyway.
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	// TODO: Does casting to an interface avoid hard references? What is a hard reference anyway, and why is avoiding hard refs important?
	// Something to investigate for later.
	auto ChompGameMode = Cast<AChompGameMode>(GameMode);
	check(ChompGameMode);

	ChompGameMode->SetGameState(PacmanGameState::Playing);

	TArray<UUserWidget *> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		GetWorld(),
		FoundWidgets,
		UGameOverWidget::StaticClass(),
		false);

	// Hide the widget.
	for (auto Widget : FoundWidgets)
	{
		Widget->RemoveFromParent();
		Widget->Destruct();
	}
	GameOverWidgetInstance = nullptr;

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(false);
	Controller->SetInputMode(FInputModeGameOnly());
}
