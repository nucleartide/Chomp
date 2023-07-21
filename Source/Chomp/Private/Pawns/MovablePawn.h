// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "MovablePawn.generated.h"

USTRUCT()
struct FMovementResult
{
	GENERATED_BODY()

	bool MovedPastTarget = false;
	float AmountMovedPast = 0.0f;
};

UCLASS()
class AMovablePawn : public APawn
{
	GENERATED_BODY()

private:
	/**
	 * The collision tags that this pawn collides with.
	 */
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

	/**
	 * Movement speed scaling factor.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 5.0f;

public:
	AMovablePawn();
	TArray<FName> GetTagsToCollideWith();
	FMovementResult MoveTowardsPoint(FGridLocation TargetGridPosition, FGridLocation TargetDirection, float DeltaTime);

private:
	void WrapAroundWorld();
};
