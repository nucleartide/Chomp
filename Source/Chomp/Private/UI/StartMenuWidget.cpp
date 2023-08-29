#include "StartMenuWidget.h"
#include "Components/Button.h"
#include "GameState/HighScoreSubsystem.h"
#include "Utils/SafeGet.h"

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

void UStartMenuWidget::Render(APlayerController* PlayerController) const
{
	// Pre-conditions.
	check(PlayerController);
	const auto WorldInstance = FSafeGet::World(PlayerController);

	// Grab references to data.
	const auto HighScoreSubsystem = WorldInstance->GetGameInstance()->GetSubsystem<UHighScoreSubsystem>();
	check(HighScoreSubsystem);

	// Set high score value.
	auto HighScoreText = FText::FromString(FString::Printf(TEXT("%d"), HighScoreSubsystem->GetHighScore()));
	HighScoreValue->SetText(HighScoreText);

	// Set high score new indicator.
	const auto IsHighScoreNew = HighScoreSubsystem->GetIsHighScoreNew();
	HighScoreNewIndicator->SetVisibility(IsHighScoreNew ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	// Set high score bonus symbol widget, if present.
	if (const auto HighScoreBonusSymbolWidget = HighScoreSubsystem->GetHighScoreBonusSymbolWidget())
	{
		const auto WidgetInstance = CreateWidget(WorldInstance, HighScoreBonusSymbolWidget);
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
