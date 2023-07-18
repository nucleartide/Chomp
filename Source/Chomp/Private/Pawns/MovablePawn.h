// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MovablePawn.generated.h"

UCLASS()
class AMovablePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMovablePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

	//
	// Custom fields/methods below:
	//

	// Pacman should wrap around when exceeding world bounds.
	void WrapAroundWorld();

	// Move this pawn around by Value.
	virtual void MoveVector(FVector2D Value, float DeltaTime);

	// Extend collision raycasts by this factor for the sake of more robust collision checks.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float Tolerance = 2.0f;

	// Reference to the ULevelLoader. Needed for bounds checks.
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	TSubclassOf<class ULevelLoader> Level;

	// Scaling factor that is fed to RInterpTo.
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float RotationInterpSpeed = 1.0f;

	/**
	 * Collision channel that determines which objects this pawn collides with. Defaults to all visible objects.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TEnumAsByte<ECollisionChannel> ObjectsToCollideWith = ECC_Visibility;
};
