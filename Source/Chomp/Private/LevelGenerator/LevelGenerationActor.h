// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChompGameState.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Templates/SubclassOf.h"
#include "LevelGenerator/LevelLoader.h"
#include "LevelGenerationActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelClearedSignature);

UCLASS()
class CHOMP_API ALevelGenerationActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Level Loader Customization")
	TSubclassOf<ULevelLoader> LevelLoader;

	UPROPERTY(EditDefaultsOnly, Category = "Instantiated Tile Customization")
	TSubclassOf<AStaticMeshActor> WallTile;

	UPROPERTY(EditDefaultsOnly, Category = "Instantiated Tile Customization")
	TSubclassOf<AStaticMeshActor> PacmanDot;
	
	UPROPERTY(EditDefaultsOnly, Category = "Instantiated Tile Customization")
	TSubclassOf<AStaticMeshActor> EnergizerDot;

	UPROPERTY(EditDefaultsOnly, Category = "Instantiated Tile Customization")
	TSubclassOf<AStaticMeshActor> GateTile;
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnLevelClearedSignature OnLevelClearedDelegate;

private:
	/**
	 * A list of generated tiles in the scene.
	 */
	UPROPERTY()
	TArray<AActor *> Tiles;

	/**
	 * Override BeginPlay() to customize initialization.
	 */
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	/**
	 * Clear any leftover tiles in the level.
	 */
	UFUNCTION(BlueprintCallable)
	void ClearLeftoverTiles();

	/**
	 * Generate tiles from the current LevelLoader's level data.
	 */
	UFUNCTION(BlueprintCallable)
	void GenerateTiles();

	/**
	 * Reset the level's tiles to that of the original level data.
	 */
	UFUNCTION(BlueprintCallable)
	void ResetStateOfEverything(EChompGameStateEnum OldState, EChompGameStateEnum NewState);
};
