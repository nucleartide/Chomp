#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionStoreSubsystem.generated.h"

class UChompSaveGame;
class ULevelDataAsset;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API USessionStoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly)
	UChompSaveGame* ChompSaveGame;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	UChompSaveGame* GetSaveGame() const;
};
