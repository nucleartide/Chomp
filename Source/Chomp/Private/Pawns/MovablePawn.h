// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AStar/MovementPath.h"
#include "GameFramework/Pawn.h"
#include "LevelGenerator/LevelLoader.h"

#include "MovablePawn.generated.h"

struct FMovementIntention;
struct FMovement;
struct FMovementResult;
struct FMoveInDirectionResult;
struct FPeriodicDotProductResult;

UCLASS()
class AMovablePawn : public APawn
{
	GENERATED_BODY()

	// The collision tags that this pawn collides with.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TArray<FName> TagsToCollideWith;

	// Scaling factor that is fed to RInterpTo.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float RotationInterpSpeed = 1.0f;

	/**
	 * Reference to the ULevelLoader. Needed for bounds checks.
	 */
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	/**
	 * Movement speed scaling factor.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 5.0f;

public:
	FMoveInDirectionResult MoveInDirection(
		TSharedPtr<FMovement> Movement,
		TSharedPtr<FMovementIntention> MovementIntention,
		const float DeltaTime) const;

	FGridLocation GetGridLocation() const;

	FVector2D GetActorLocation2D() const;

	FMovementResult MoveAlongPath(
		FMovementPath* MovementPath,
		const float DeltaTime) const;

	// Check whether a Pawn at Location can travel 1 unit in Direction.
	bool CanTravelInDirection(FVector Location, FGridLocation Direction) const;

private:
	static FVector WrapAroundWorld(FVector Location, const ULevelLoader* LevelInstance);

	FRotator ComputeNewRotation(const FVector& CurrentLocation, const FVector& NewLocation, float DeltaTime) const;

	static FPeriodicDotProductResult ComputeDotProduct(
		const FGridLocation& MovementDirection,
		const FVector& PostMovementLocation,
		const FGridLocation& Target,
		const ULevelLoader* LevelInstance
	);

	static FVector2D MinDifferenceVector(FVector From, FVector To, const ULevelLoader* LevelInstance);
};
