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

	// Reference to the ULevelLoader. Needed for bounds checks.
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	TSubclassOf<ULevelLoader> Level;

	// Movement speed scaling factor.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float MovementSpeed = 5.0f;

public:
	// Move in a certain direction. Used by the player controller.
	FMoveInDirectionResult MoveInDirection(
		TSharedPtr<FMovement> Movement,
		TSharedPtr<FMovementIntention> MovementIntention,
		const float DeltaTime
	) const;

	// Helper for getting the current grid position.
	FGridLocation GetGridLocation() const;

	// Move along a path. Used by the AI controller.
	FMovementResult MoveAlongPath(
		FMovementPath* MovementPath,
		const float DeltaTime
	) const;

	// Check whether a Pawn at Location can travel 1 unit in Direction.
	bool CanTravelInDirection(FVector Location, FGridLocation Direction) const;

private:
	// Wrap a Location around the ULevelLoader bounds.
	static FVector WrapAroundWorld(FVector Location, const ULevelLoader* LevelInstance);

	// Compute a new rotation given the movement direction.
	FRotator ComputeNewRotation(const FVector& CurrentLocation, const FVector& NewLocation, float DeltaTime) const;

	// Compute the dot product between MovementDirection, and (Target - PostMovementLocation).
	//
	// Note that this method handles wrap-around correctly.
	static FPeriodicDotProductResult ComputeDotProduct(
		const FGridLocation& MovementDirection,
		const FVector& PostMovementLocation,
		const FGridLocation& Target,
		const ULevelLoader* LevelInstance
	);

	// Find the minimum difference vector (To - From), while taking into account that the ULevelLoader wraps around.
	static FVector2D MinDifferenceVector(FVector From, FVector To, const ULevelLoader* LevelInstance);
};
