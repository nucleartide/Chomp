#include "StartMenuWidget.h"
#include "Components/Button.h"
#include "Utils/Debug.h"

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	StartButton->OnClicked.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartGame);
	OptionsButton->OnClicked.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsButtonClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitGame);
	
	StartButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartHover);
	// OptionsButton->OnClicked.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsButtonClicked);
	// QuitButton->OnClicked.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitGame);
}

void UStartMenuWidget::HandleStartGame()
{
	DEBUG_LOG(TEXT("start game clicked"));
}

void UStartMenuWidget::HandleOptionsButtonClicked()
{
	DEBUG_LOG(TEXT("options button clicked"));
}

void UStartMenuWidget::HandleQuitGame()
{
	DEBUG_LOG(TEXT("quit game clicked"));
}

void UStartMenuWidget::HandleStartHover()
{
	// Find the dot text.
	// ...

	// Enable the dot text.
	// ...
}
