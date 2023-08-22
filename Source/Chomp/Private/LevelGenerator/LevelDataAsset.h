#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelDataAsset.generated.h"

class ABonusSymbol;

UCLASS(BlueprintType)
class CHOMP_API ULevelDataAsset : public UDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	TSubclassOf<ABonusSymbol> BonusSymbol;

	UPROPERTY(EditInstanceOnly)
	UMaterialInterface* BonusSymbolRenderTarget;
	
public:
	UMaterialInterface* GetBonusSymbolRenderTarget() const;
};
