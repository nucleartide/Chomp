#include "UI/UIManager.h"
#include "ChompGameState.h"
#include "CoreGlobals.h"
#include "LevelIndicatorWidget.h"
#include "LivesWidget.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GameOverWidget.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

// [x] M - use SetPaused
// [x] V - this is done
//   [x] create a new HUDScreen
// [o] C - starting
//   [ ] refactor UIManager to write less code in C++
// ---
//   [ ] on p, show pause screen
//   [ ] see user stories in github
// [ ] bonus: options button

void AUIManager::BeginPlay()
{
	Super::BeginPlay();

	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
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
	// Pre-conditions.
	check(OldState != NewState);

	// Early return.
	if (NewState != EChompGameStateEnum::GameOverLose)
		return;

	// Instantiate game over widget.
	DEBUG_LOG(TEXT("Player died, showing game over (lose) UI..."))
	const auto World = FSafeGet::World(this);
	const auto WidgetInstance = CreateWidget(World, GameOverLoseWidget);
	check(WidgetInstance);
	GameOverWidgetInstance = Cast<UGameOverWidget>(WidgetInstance);
	check(GameOverWidgetInstance);
	GameOverWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIManager::HandleRestartGameClicked);
	GameOverWidgetInstance->AddToViewport();

	// Show and enable mouse cursor.
	const auto Controller = FSafeGet::PlayerController(this, 0);
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

// ReSharper disable once CppMemberFunctionMayBeConst
void AUIManager::HandleLivesChanged(const int NumberOfLives)
{
}
