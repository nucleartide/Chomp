// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/MovablePawn.h"
#include "ChompPawn.generated.h"

UCLASS()
class AChompPawn : public AMovablePawn
{
	GENERATED_BODY()

protected:
	virtual void NotifyActorBeginOverlap(AActor *Other) override;
};
