// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PacmanGameMode.generated.h"

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, PacmanGameState, GameState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameRestartedSignature);

// TODO: can probably just remove this class later
UCLASS()
class KIWIPIZZA_API APacmanGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameRestartedSignature OnGameRestartedDelegate;
};
