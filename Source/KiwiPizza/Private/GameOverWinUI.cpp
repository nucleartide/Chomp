#include "GameOverWinUI.h"
#include "Debug.h"
#include "Components/Button.h"

void UGameOverWinUI::NativeConstruct()
{
    Super::NativeConstruct();
    RestartGameButton->OnReleased.AddUniqueDynamic(this, &UGameOverWinUI::HandleRestartGame);
}

void UGameOverWinUI::HandleRestartGame()
{
    DEBUG_LOG(TEXT("Should restart game."));
}
