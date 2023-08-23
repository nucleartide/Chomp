#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurrentLevelManager.generated.h"

class ULevelSequenceDataAsset;

UCLASS(BlueprintType)
class CHOMP_API ACurrentLevelManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	ULevelSequenceDataAsset* LevelSequence;

	UPROPERTY(VisibleInstanceOnly)
	int CurrentLevel = 0;
};
