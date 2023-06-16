// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Templates/SubclassOf.h"
#include "LevelLoader.h"
#include "LevelGenerationActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelClearedSignature);

UCLASS()
class KIWIPIZZA_API ALevelGenerationActor : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION(BlueprintCallable)
	void RegenerateDots();

	UPROPERTY(EditDefaultsOnly, Category = "Wall Tile Customization")
	TSubclassOf<AStaticMeshActor> WallTile;

	UPROPERTY(EditDefaultsOnly, Category = "Pacman Dot Customization")
	TSubclassOf<AStaticMeshActor> PacmanDot;

	UPROPERTY(EditDefaultsOnly, Category = "Level Loader Customization")
	TSubclassOf<ULevelLoader> LevelLoader;

	int NumDotsGenerated = 0;

	UFUNCTION(BlueprintCallable)
	void HandleDotConsumption();

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnLevelClearedSignature OnLevelClearedDelegate;
};
