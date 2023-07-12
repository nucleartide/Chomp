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

// Dot lifecycle events.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsResetSignature, int, NumberOfDots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotConsumedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);

// Player death event.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChompDiedSignature);

// State change events.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature, ChompGameState, OldState, ChompGameState, NewState);

UCLASS()
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void ResetDots(int NumberOfDots);
	void ConsumeDot();
	void NotifyPlayerDeath();
	void TransitionTo(ChompGameState NewState);

public:

	/**
	 * Note that only ChompGameState should be invoking .Broadcast() on these delegates.
	 *
	 * If you want to call a delegate from outside ChompGameState, replace the call with a call to a public method instead.
	 */

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsResetSignature OnDotsResetDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotConsumedSignature OnDotConsumedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsClearedSignature OndotsClearedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnChompDiedSignature OnChompDiedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

private:
	int Score = 0;
	int NumberOfDotsRemaining = 0;
	ChompGameState GameState = ChompGameState::Playing;
};
