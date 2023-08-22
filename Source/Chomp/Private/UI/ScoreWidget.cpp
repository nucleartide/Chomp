#include "UI/ScoreWidget.h"
#include "Components/TextBlock.h"
#include "ChompGameState.h"

void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    const auto ChompGameState = GetWorld()->GetGameState<AChompGameState>();
    ChompGameState->OnScoreUpdated.AddUniqueDynamic(this, &UScoreWidget::HandleScoreUpdated);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UScoreWidget::HandleScoreUpdated(int Score)
{
    check(ScoreTextBlock);
    auto NewScoreText = FText::FromString(FString::Printf(TEXT("%d"), Score));
    ScoreTextBlock->SetText(NewScoreText);
}
