// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelLoader.h"
#include "PacmanPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPacmanDiedSignature);

UCLASS()
class APacmanPawn : public APawn
{
	GENERATED_BODY()

public:
	APacmanPawn();

protected:
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor *Other) override;
	virtual void NotifyActorEndOverlap(AActor *Other) override;

public:
	/**
	 * Overridden tick function.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Pacman should wrap around when exceeding world bounds.
	 */
	void WrapAroundWorld();

	/**
	 *  Move this pawn around by Value.
	 */
	virtual void MoveVector(FVector2D Value);

	/**
	 * Extend collision raycasts by this factor for the sake of more robust collision checks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Tolerance Customization")
	float Tolerance = 2.0f;

	/**
	 * Reference to the ULevelLoader. Needed for bounds checks.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Loaded Level")
	TSubclassOf<ULevelLoader> Level;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPacmanDiedSignature OnPacmanDiedDelegate;
};
