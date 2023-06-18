#include "UIActor.h"
#include "Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameOverWinWidget.h"
#include "PacmanGameMode.h"

AUIActor::AUIActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUIActor::BeginPlay()
{
	Super::BeginPlay();
	LevelGenerator->OnLevelClearedDelegate.AddUniqueDynamic(this, &AUIActor::HandleDotsCleared);
}

void AUIActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUIActor::HandleDotsCleared()
{
	DEBUG_LOG(TEXT("Dots cleared, showing game over win UI..."))

    auto World = GetWorld();
	check(World);

	auto WidgetInstance = CreateWidget(World, GameOverWinWidgetClass);
	check(WidgetInstance);

	{
		GameOverWinWidgetInstance = Cast<UGameOverWinWidget>(WidgetInstance);
		check(GameOverWinWidgetInstance);
		GameOverWinWidgetInstance->OnRestartGameClickedDelegate.AddUniqueDynamic(this, &AUIActor::HandleRestartGameClicked);
	}

	WidgetInstance->AddToViewport();

	auto ViewportClient = World->GetGameViewport();
	check(ViewportClient);

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
    Controller->SetInputMode(FInputModeGameAndUI());

	// TODO: Figure out how to do this: https://www.youtube.com/watch?v=bWoew0fa_xA
}

void AUIActor::HandleRestartGameClicked()
{
	DEBUG_LOG(TEXT("Handling game restarted in UIActor..."))

	// By reaching out to the current game mode, this class becomes coupled to PacmanGameMode.
	// That's okay. This class is intended to be game-specific anyway.
	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	PacmanGameMode->SetGameState(PacmanGameState::Playing);

	// TODO: Hide the widget.
	GameOverWinWidgetInstance->RemoveFromParent();
	GameOverWinWidgetInstance->Destruct();
	GameOverWinWidgetInstance = nullptr;

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(false);
    Controller->SetInputMode(FInputModeGameOnly());
}
