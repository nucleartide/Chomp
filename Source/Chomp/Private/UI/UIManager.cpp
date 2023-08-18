#include "UI/UIManager.h"
#include "Utils/Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UI/GameOverWidget.h"
#include "ChompGameState.h"
#include "LivesWidget.h"
#include "Components/HorizontalBox.h"
#include "UE5Coro.h"
#include "Utils/SafeGet.h"

AUIManager::AUIManager(): AActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIManager::BeginPlay()
{
	Super::BeginPlay();

	const auto World = FSafeGet::World(this);

	ScoreWidgetInstance = CreateWidget(World, ScoreWidget);
	check(ScoreWidgetInstance);
	ScoreWidgetInstance->AddToViewport();

	LivesWidgetInstance = CreateWidget(World, LivesWidget);
	check(LivesWidgetInstance);
	LivesWidgetInstance->AddToViewport();

	const auto GameState = GetWorld()->GetGameState<AChompGameState>();
	GameState->OnDotsCleared.AddUniqueDynamic(this, &AUIManager::HandleDotsCleared);
	GameState->OnGameStateChanged.AddUniqueDynamic(this, &AUIManager::HandlePlayerDeath);
	GameState->OnLivesChanged.AddUniqueDynamic(this, &AUIManager::HandleLivesChanged);
}

void AUIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const auto GameState = GetWorld()->GetGameState<AChompGameState>();
	GameState->OnDotsCleared.RemoveDynamic(this, &AUIManager::HandleDotsCleared);
	GameState->OnGameStateChanged.RemoveDynamic(this, &AUIManager::HandlePlayerDeath);
	GameState->OnLivesChanged.RemoveDynamic(this, &AUIManager::HandleLivesChanged);

	ScoreWidgetInstance->RemoveFromParent();
	LivesWidgetInstance->RemoveFromParent();

	ScoreWidgetInstance = nullptr;
	LivesWidgetInstance = nullptr;
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

UE5Coro::TCoroutine<> AsyncAction()
{
	co_await UE5Coro::Latent::Seconds(1.0);
	DEBUG_LOG(TEXT("test blah blah"));
}

void AUIManager::HandlePlayerDeath(const EChompGameStateEnum OldState, const EChompGameStateEnum NewState)
{
	const auto DidLose = OldState != NewState && NewState == EChompGameStateEnum::GameOverLose;
	if (!DidLose)
		return;

	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	ChompGameState->LoseLife();
	
	// if (ChompGameState->)

	// [x] add coros to project
	// [x] implement the below
	// if number of lives is zero, continue
	// else,
	//     decrement one life
	//     kick off a timer for 3s - use coros
	// ---
	//     once timer is elapsed, reset the round (keep the dots, reset the ghost state, reset the player)

	AsyncAction();

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

void AUIManager::HandleLivesChanged(const int NumberOfLives)
{
	const auto LivesWidgetRef = Cast<ULivesWidget>(LivesWidgetInstance);
	check(LivesWidgetRef);

	const auto World = FSafeGet::World(this);

	LivesWidgetRef->LivesContainer->ClearChildren();

	for (auto i = 0; i < NumberOfLives; i++)
	{
		const auto LifeWidgetInstance = CreateWidget(World, LivesWidgetRef->LifeWidget);
		LifeWidgetInstance->SetPadding(FMargin(LivesWidgetRef->HorizontalPadding, 0.0));
		LivesWidgetRef->LivesContainer->AddChild(LifeWidgetInstance);
	}
}
