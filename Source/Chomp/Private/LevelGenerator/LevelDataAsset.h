#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

class ULevelSymbolWidget;
class ABonusSymbol;

UCLASS(BlueprintType)
class CHOMP_API ULevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly)
	TSubclassOf<ABonusSymbol> BonusSymbol;

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<ULevelSymbolWidget> BonusSymbolWidget;
};
