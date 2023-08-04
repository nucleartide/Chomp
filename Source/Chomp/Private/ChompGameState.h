#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Utils/FieldWithLastUpdatedTime.h"

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
enum class EChompGamePlayingSubstate : uint8
{
	None,
	Scatter,
	Chase,
	Frightened,
};

/**
 * An FWave is a period of time during which a particular EChompGamePlayingSubstate is active.
 */
USTRUCT()
struct FWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EChompGamePlayingSubstate PlayingState;

	UPROPERTY(EditAnywhere)
	double Duration;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdatedSignature, int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature,
                                             EChompGameState, OldState,
                                             EChompGameState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChangedSignature,
                                             EChompGamePlayingSubstate, OldSubstate,
                                             EChompGamePlayingSubstate, NewSubstate);

UCLASS(Blueprintable)
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TArray<FWave> Waves;

	UPROPERTY(VisibleAnywhere)
	int Score = 0;

	UPROPERTY(VisibleAnywhere)
	int NumberOfDotsRemaining = 0;

	UPROPERTY(VisibleAnywhere)
	FIntFieldWithLastUpdatedTime NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime(0, nullptr);
	
	UPROPERTY(VisibleAnywhere)
	EChompGameState GameState = EChompGameState::None;

	UPROPERTY(VisibleAnywhere)
	EChompGamePlayingSubstate LastKnownGamePlayingSubstate = EChompGamePlayingSubstate::None;

	// The time (as reported by UWorld::GetTimeSeconds) when the game entered into an EChompGameState::Playing state.
	UPROPERTY(VisibleAnywhere)
	float GameStartTime = 0.0f;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsClearedSignature OnDotsClearedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsConsumedUpdatedSignature OnDotsConsumedUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChangedSignature OnGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnScoreUpdatedSignature OnScoreUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGamePlayingStateChangedSignature OnGamePlayingStateChangedDelegate;

	AChompGameState();

	void ResetDots(int NumberOfDots);

	void ConsumeDot();

	EChompGameState GetEnum() const;

	int GetScore() const;

	EChompGamePlayingSubstate GetPlayingSubstate() const;

	FIntFieldWithLastUpdatedTime GetNumberOfDotsConsumed() const;

	void LoseGame();

	void StartGame();

	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	void UpdateScore(int NewScore);

	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);

	float GetTimeSinceStart() const;

	void TransitionTo(EChompGameState NewState);
};
