// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PacmanGameMode.generated.h"

UENUM(BlueprintType)
enum PacmanGameState
{
	Playing,
	GameOverWin,
	GameOverLose,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, PacmanGameState, GameState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameRestartedSignature);

UCLASS()
class KIWIPIZZA_API APacmanGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	PacmanGameState GameState = Playing;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameRestartedSignature OnGameRestartedDelegate;
};
