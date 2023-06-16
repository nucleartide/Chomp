#include "UIActor.h"
#include "Debug.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

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

	auto GameOverWinWidget = CreateWidget(World, GameOverWinUI);
	check(GameOverWinWidget);

	GameOverWinWidget->AddToViewport();

	auto ViewportClient = World->GetGameViewport();
	check(ViewportClient);

	auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->SetShowMouseCursor(true);
    Controller->SetInputMode(FInputModeGameAndUI());

	// TODO: Figure out how to do this: https://www.youtube.com/watch?v=bWoew0fa_xA
}
