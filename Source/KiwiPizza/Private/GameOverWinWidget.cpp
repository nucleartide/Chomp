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

    // Reach out to the game mode, and invoke the restart game delegate.
    OnRestartGameClickedDelegate.Broadcast();

    // Reset the player's position.
    // ...

    // Hide the game over UI.
    // ...

    // Regenerate the level's dots.
    // ...
}
