// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Templates/SubclassOf.h"
#include "Actors/LevelLoader.h"
#include "LevelGenerationActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelClearedSignature);

UCLASS()
class KIWIPIZZA_API ALevelGenerationActor : public AActor
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
	TSubclassOf<AStaticMeshActor> GateTile;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnLevelClearedSignature OnLevelClearedDelegate;

private:
	/**
	 * A list of generated tiles in the scene.
	 */
	TArray<AActor *> Tiles;

	/**
	 * A count of the number of active dots in the scene.
	 */
	int NumberOfDotsRemaining = 0;

	/**
	 * Override BeginPlay() to customize initialization.
	 */
	void BeginPlay() override;

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
	void ResetTiles();

	/**
	 * Handle when a dot in the level is consumed.
	 */
	UFUNCTION(BlueprintCallable)
	void HandleDotConsumption();
};
