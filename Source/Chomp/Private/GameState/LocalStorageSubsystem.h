#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalStorageSubsystem.generated.h"

class UChompSaveGame;
class ULevelDataAsset;

UCLASS(BlueprintType)
class CHOMP_API ULocalStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	UChompSaveGame* ChompSaveGame;
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable)
	UChompSaveGame* GetSaveGame() const;
};
