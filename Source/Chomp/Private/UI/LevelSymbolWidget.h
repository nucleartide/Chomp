#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "LevelSymbolWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CHOMP_API ULevelSymbolWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UImage* LevelSymbolImage;
};
