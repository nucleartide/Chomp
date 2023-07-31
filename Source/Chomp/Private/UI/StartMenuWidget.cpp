#include "StartMenuWidget.h"
#include "Components/Button.h"
#include "Utils/Debug.h"

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StartButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartHover);
	StartButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartUnhover);

	OptionsButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsHover);
	OptionsButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsUnhover);

	QuitButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitHover);
	QuitButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitUnhover);
}

void UStartMenuWidget::HandleStartHover()
{
	StartButtonBullet->SetVisibility(ESlateVisibility::Visible);
}

void UStartMenuWidget::HandleStartUnhover()
{
	StartButtonBullet->SetVisibility(ESlateVisibility::Hidden);
}

void UStartMenuWidget::HandleOptionsHover()
{
	OptionsButtonBullet->SetVisibility(ESlateVisibility::Visible);
}

void UStartMenuWidget::HandleOptionsUnhover()
{
	OptionsButtonBullet->SetVisibility(ESlateVisibility::Hidden);
}

void UStartMenuWidget::HandleQuitHover()
{
	QuitButtonBullet->SetVisibility(ESlateVisibility::Visible);
}

void UStartMenuWidget::HandleQuitUnhover()
{
	QuitButtonBullet->SetVisibility(ESlateVisibility::Hidden);
}
