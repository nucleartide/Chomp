// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/MovablePawn.h"
#include "ChompPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPacmanDiedSignature);

UCLASS()
class AChompPawn : public AMovablePawn
{
	GENERATED_BODY()

public:
	AChompPawn();

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
	virtual void MoveVector(FVector2D Value, float DeltaTime) override;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPacmanDiedSignature OnPacmanDiedDelegate;
};