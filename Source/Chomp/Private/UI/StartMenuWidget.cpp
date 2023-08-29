#include "StartMenuWidget.h"
#include "Components/Button.h"

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Pre-conditions.
	check(BonusSymbolWidget);

	StartButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartHover);
	StartButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleStartUnhover);

	OptionsButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsHover);
	OptionsButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleOptionsUnhover);

	QuitButton->OnHovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitHover);
	QuitButton->OnUnhovered.AddUniqueDynamic(this, &UStartMenuWidget::HandleQuitUnhover);
}

void UStartMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();

	StartButton->OnHovered.RemoveDynamic(this, &UStartMenuWidget::HandleStartHover);
	StartButton->OnUnhovered.RemoveDynamic(this, &UStartMenuWidget::HandleStartUnhover);

	OptionsButton->OnHovered.RemoveDynamic(this, &UStartMenuWidget::HandleOptionsHover);
	OptionsButton->OnUnhovered.RemoveDynamic(this, &UStartMenuWidget::HandleOptionsUnhover);

	QuitButton->OnHovered.RemoveDynamic(this, &UStartMenuWidget::HandleQuitHover);
	QuitButton->OnUnhovered.RemoveDynamic(this, &UStartMenuWidget::HandleQuitUnhover);
}

void UStartMenuWidget::Render(UWorld* WorldInstance) const
{
	// Pre-conditions.
	check(WorldInstance);
	
	const auto DummyHighScore = FText::FromString(TEXT("blah"));
	HighScoreValue->SetText(DummyHighScore);
	HighScoreNewIndicator->SetVisibility(IsHighScoreNew ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
	if (BonusSymbolWidget)
	{
		// Then render instance of bonus symbol into box.
		const auto WidgetInstance = CreateWidget(WorldInstance, BonusSymbolWidget);
		check(WidgetInstance);
		HighScoreLevelSymbolBox->AddChild(WidgetInstance);
	}
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
