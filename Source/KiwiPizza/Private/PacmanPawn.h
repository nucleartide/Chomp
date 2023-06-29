// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelLoader.h"
#include "MovablePawn.h"
#include "PacmanPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPacmanDiedSignature);

UCLASS()
class APacmanPawn : public AMovablePawn
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
	 *  Move this pawn around by Value.
	 */
	virtual void MoveVector(FVector2D Value) override;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPacmanDiedSignature OnPacmanDiedDelegate;
};
