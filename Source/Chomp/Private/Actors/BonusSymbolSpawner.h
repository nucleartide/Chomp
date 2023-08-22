#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BonusSymbolSpawner.generated.h"

class ULevelLoader;
class ABonusSymbol;

UCLASS()
class CHOMP_API ABonusFruitSpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int FirstSymbolDotThreshold = 70;

	UPROPERTY(EditDefaultsOnly)
	int SecondSymbolDotThreshold = 170;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABonusSymbol> BonusSymbolToSpawn;

	UPROPERTY(VisibleInstanceOnly)
	ABonusSymbol* SpawnedBonusSymbol;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULevelLoader> LevelLoader;

	UFUNCTION()
	void HandleDotsConsumedChanged(int DotsConsumed);

	ABonusSymbol* SpawnBonusSymbol() const;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
