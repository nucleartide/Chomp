#include "UI/ScoreWidget.h"

#include "Components/TextBlock.h"

#include "ChompGameState.h"
#include "Utils/Debug.h"

void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    auto ChompGameState = GetWorld()->GetGameState<AChompGameState>();
    ChompGameState->OnScoreUpdatedDelegate.AddUniqueDynamic(this, &UScoreWidget::HandleScoreUpdated);
}

void UScoreWidget::HandleScoreUpdated(int Score)
{
    check(ScoreTextBlock);
    auto NewScoreText = FText::FromString(FString::Printf(TEXT("%d"), Score));
    ScoreTextBlock->SetText(NewScoreText);
}
