#include "GameOverWinWidget.h"
#include "Debug.h"
#include "Components/Button.h"

void UGameOverWinWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RestartGameButton->OnReleased.AddUniqueDynamic(this, &UGameOverWinWidget::HandleRestartGame);
}

void UGameOverWinWidget::HandleRestartGame()
{
    DEBUG_LOG(TEXT("Handling restart game..."));
    OnRestartGameClickedDelegate.Broadcast();
}
