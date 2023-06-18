// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PacmanGameMode.generated.h"

UENUM(BlueprintType)
enum class PacmanGameState : uint8
{
	Playing,
	GameOverWin,
	GameOverLose,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameRestartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, PacmanGameState, GameState);

UCLASS()
class KIWIPIZZA_API APacmanGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	PacmanGameState GameState = PacmanGameState::Playing;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameRestartedSignature OnGameRestartedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	UFUNCTION()
	void SetGameState(PacmanGameState NewState);
};
