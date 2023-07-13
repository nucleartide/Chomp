#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "ChompGameState.generated.h"

UENUM(BlueprintType)
enum class EChompGameState : uint8
{
	None,
	Playing,
	GameOverWin,
	GameOverLose,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature, EChompGameState, OldState, EChompGameState, NewState);

UENUM()
enum class EChompGamePlayingState : uint8
{
	None,
	Scatter,
	Chase,
	Frightened,
};

/**
 * An FWave is a period of time during which a particular EChompGamePlayingState is active.
 */
USTRUCT()
struct FWave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
	EChompGamePlayingState PlayingState;

    UPROPERTY(EditAnywhere)
	double Duration;
};

UCLASS()
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/**
	 * Public methods.
	 */

	void ResetDots(int NumberOfDots);
	void ConsumeDot();
	void NotifyPlayerDeath();
	void TransitionTo(EChompGameState NewState);
	EChompGameState GetEnum();
	int GetScore();
	EChompGamePlayingState GetCurrentWave();

public:

	/**
	 * Public delegates.
	 *
	 * Note that only ChompGameState should be invoking .Broadcast() on these delegates.
	 *
	 * If you want to call a delegate from outside ChompGameState, replace the call with a call to a public method instead.
	 */

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsClearedSignature OnDotsClearedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	/**
	 * Used for callbacks that you want executed after those assigned to OnGameStateChangedDelegate.
	 *
	 * Example:
	 *   1. Reset the player's position in OnGameStateChangedDelegate
	 *   2. Reset the level's dots in OnLateGameStateChangedDelegate
	 */
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnLateGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnScoreUpdatedSignature OnScoreUpdatedDelegate;

private:

	/**
	 * Properties.
	 */

	/**
	 * The score amount that should be awarded upon dot consumption.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TArray<FWave> Waves;

	/**
	 * Fields.
	 */

	int Score = 0;
	int NumberOfDotsRemaining = 0;
	EChompGameState GameState = EChompGameState::Playing;

	/**
	 * Behavior.
	 */

	void UpdateScore(int NewScore);
	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);
};
