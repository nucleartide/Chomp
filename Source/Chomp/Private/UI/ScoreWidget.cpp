#include "UI/ScoreWidget.h"

#include "Components/TextBlock.h"

#include "ChompGameState.h"
#include "Utils/Debug.h"

void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();

    auto ChompGameState = GetWorld()->GetGameState<AChompGameState>();
    check(ChompGameState);

    ChompGameState->OnScoreUpdatedDelegate.AddUniqueDynamic(this, &UScoreWidget::HandleScoreUpdated);
}

void UScoreWidget::HandleScoreUpdated(int NewScore)
{
    DEBUG_LOG(TEXT("New score: %d"), NewScore);
    check(ScoreText);

    auto NewScoreText = FText::FromString(FString::Printf(TEXT("%d"), NewScore));
    ScoreText->SetText(NewScoreText);
}
