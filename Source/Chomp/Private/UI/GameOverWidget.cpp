#include "UI/GameOverWidget.h"
#include "Utils/Debug.h"
#include "Components/Button.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RestartGameButton->OnReleased.AddUniqueDynamic(this, &UGameOverWidget::HandleRestartGame);
}

void UGameOverWidget::HandleRestartGame()
{
    DEBUG_LOG(TEXT("Handling restart game..."));
    OnRestartGameClickedDelegate.Broadcast();
}
