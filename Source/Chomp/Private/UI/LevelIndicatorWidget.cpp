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
		// Create an image.
		const auto WidgetInstance = CreateWidget<UUserWidget>(WorldInstance, LevelSymbolWidget);
		check(WidgetInstance);

		const auto LevelSymbolInstance = Cast<ULevelSymbolWidget>(WidgetInstance);
		check(LevelSymbolInstance);

		// Set the image's render target from the current loop iteration's level.
		LevelSymbolInstance->LevelSymbolImage->SetBrushFromMaterial(Level->GetBonusSymbolRenderTarget());

		// Add the image to the horizontal box.
		LevelIndicatorContainer->AddChild(LevelSymbolInstance);
	}
}
