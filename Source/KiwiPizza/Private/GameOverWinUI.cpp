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
