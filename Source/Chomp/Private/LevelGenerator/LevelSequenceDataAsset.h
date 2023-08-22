#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelSequenceDataAsset.generated.h"

class ULevelDataAsset;

UCLASS(BlueprintType)
class CHOMP_API ULevelSequenceDataAsset : public UDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly)
	TArray<ULevelDataAsset*> LevelSequence;
};
