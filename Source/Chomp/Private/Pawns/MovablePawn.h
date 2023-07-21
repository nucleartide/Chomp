// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "MovablePawn.generated.h"

UCLASS()
class AMovablePawn : public APawn
{
	GENERATED_BODY()

private:
	/**
	 * Extend collision raycasts by this factor for the sake of more robust collision checks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float Tolerance = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TArray<FName> TagsToCollideWith;

	/**
	 * Scaling factor that is fed to RInterpTo.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float RotationInterpSpeed = 1.0f;

	/**
	 * Reference to the ULevelLoader. Needed for bounds checks.
	 */
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	BlockingEntity ExcludedEntities = BlockingEntity::WallsOnly;

public:
	AMovablePawn();
	BlockingEntity GetExcludedEntities();
	TArray<FName> GetTagsToCollideWith();
	bool MoveTowardsPoint(FGridLocation TargetGridPosition, FGridLocation TargetDirection, float DeltaTime);
};
