#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

class ULevelSymbolWidget;
class ABonusSymbol;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API ULevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	TSubclassOf<ABonusSymbol> BonusSymbol;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	TSubclassOf<ULevelSymbolWidget> BonusSymbolWidget;
};
