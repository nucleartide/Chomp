#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "ChompGameState.generated.h"

UENUM(BlueprintType)
enum class ChompGameState : uint8
{
	Playing,
	GameOverWin,
	GameOverLose,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsDecrementedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsResetSignature, int, NumberOfDots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameRestartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChangedSignature, PacmanGameState, GameState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPacmanDiedSignature);

UCLASS()
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnScoreUpdatedSignature OnScoreUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsDecrementedSignature OnDotsDecrementedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsDecrementedSignature OnDotsClearedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsResetSignature OnDotsResetDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameRestartedSignature OnGameRestartedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPacmanDiedSignature OnPacmanDiedDelegate;

	/**
	 * Update score and number of dots remaining.
	 */
	void ConsumeDot();

private:
	int Score = 0;
	int NumberOfDotsRemaining = 0;
	ChompGameState GameState = ChompGameState::Playing;
};
