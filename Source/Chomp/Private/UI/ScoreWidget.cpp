#include "UI/ScoreWidget.h"
#include "ChompGameState.h"
#include "Components/TextBlock.h"

void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    const auto ChompGameState = GetWorld()->GetGameState<AChompGameState>();
    ChompGameState->OnScoreUpdated.AddUniqueDynamic(this, &UScoreWidget::UpdateScoreText);
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void UScoreWidget::UpdateScoreText(int Score)
{
    check(ScoreTextBlock);
    auto NewScoreText = FText::FromString(FString::Printf(TEXT("%d"), Score));
    ScoreTextBlock->SetText(NewScoreText);
}
