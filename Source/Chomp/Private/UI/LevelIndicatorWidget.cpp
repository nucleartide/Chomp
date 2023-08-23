#include "LevelIndicatorWidget.h"

#include "LevelSymbolWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "LevelGenerator/LevelDataAsset.h"
#include "LevelGenerator/LevelSequenceDataAsset.h"

void ULevelIndicatorWidget::RenderLastThreeLevelSymbols(UWorld* WorldInstance) const
{
	for (const auto LastThreeLevels = LevelSequence->LastThree(); const auto Level : LastThreeLevels)
	{
		const auto WidgetInstance = CreateWidget<UUserWidget>(WorldInstance, Level->BonusSymbolWidget);
		check(WidgetInstance);
		LevelIndicatorContainer->AddChild(WidgetInstance);
	}
}
