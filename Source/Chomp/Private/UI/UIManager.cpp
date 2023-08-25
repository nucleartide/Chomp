#include "UI/UIManager.h"
#include "Utils/Debug.h"
#include "Kismet/GameplayStatics.h"
#include "CoreGlobals.h"
#include "GameFramework/PlayerController.h"
#include "UI/GameOverWidget.h"
#include "ChompGameState.h"
#include "LevelIndicatorWidget.h"
#include "LivesWidget.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/GameUserSettings.h"
#include "Utils/SafeGet.h"

AUIManager::AUIManager(): AActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIManager::BeginPlay()
{
	Super::BeginPlay();


		const auto GameUserSettings = UGameUserSettings::GetGameUserSettings();
    	GameUserSettings->SetOverallScalabilityLevel(1);
    	GameUserSettings->SetScreenResolution(GameUserSettings->GetDesktopResolution());
    	GameUserSettings->SetResolutionScaleNormalized(1.0);
    	GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
    	GameUserSettings->ApplySettings(false);
	DEBUG_LOG(TEXT("%s"), *GGameUserSettingsIni);
	DEBUG_LOG(TEXT("test test test"));

	const auto World = FSafeGet::World(this);

	ScoreWidgetInstance = CreateWidget(World, ScoreWidget);
	check(ScoreWidgetInstance);
	ScoreWidgetInstance->AddToViewport();

	LivesWidgetInstance = CreateWidget(World, LivesWidget);
	check(LivesWidgetInstance);
	LivesWidgetInstance->AddToViewport();
	
	LevelIndicatorWidgetInstance = CreateWidget<ULevelIndicatorWidget>(World, LevelIndicatorWidget);
	check(LevelIndicatorWidgetInstance);
	LevelIndicatorWidgetInstance->AddToViewport();
	LevelIndicatorWidgetInstance->RenderLastThreeLevelSymbols(World);

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
	LevelIndicatorWidgetInstance->RemoveFromParent();

	ScoreWidgetInstance = nullptr;
	LivesWidgetInstance = nullptr;
	LevelIndicatorWidgetInstance = nullptr;
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
