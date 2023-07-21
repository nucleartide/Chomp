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

/**
 * When in the EChompGameState::Playing state, these would be the sub-states.
 */
UENUM()
enum class EChompGamePlayingState : uint8
{
	None,
	Scatter,
	Chase,
	Frightened,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdatedSignature, int, NewDotsConsumed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature, EChompGameState, OldState, EChompGameState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChangedSignature, EChompGamePlayingState, OldSubstate, EChompGamePlayingState, NewSubstate);

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

	AChompGameState();

	void ResetDots(int NumberOfDots);

	void ConsumeDot();

	EChompGameState GetEnum();

	int GetScore();

	EChompGamePlayingState GetPlayingSubstate();

	/**
	 * Convenience wrapper around TransitionTo().
	 */
	void LoseGame();

	/**
	 * Convenience wrapper around TransitionTo().
	 */
	void StartGame();

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

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsConsumedUpdatedSignature OnDotsConsumedUpdatedDelegate;

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

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGamePlayingStateChangedSignature OnGamePlayingStateChangedDelegate;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

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
	int NumberOfDotsConsumed = 0;
	EChompGameState GameState = EChompGameState::None;
	EChompGamePlayingState LastKnownGamePlayingState = EChompGamePlayingState::None;

	/**
	 * The time (as reported by UWorld::GetTimeSeconds) when the game entered into an EChompGameState::Playing state.
	 */
	float GameStartTime = 0.0f;

	/**
	 * Behavior.
	 */

	void UpdateScore(int NewScore);
	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);
	void UpdateNumberOfDotsConsumed(int NewNumberOfDotsConsumed);
	float GetTimeSinceStart();
	void TransitionTo(EChompGameState NewState);
};
