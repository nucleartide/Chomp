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
enum class EChompGamePlayingSubstate : uint8
{
	None,
	Scatter,
	Chase,
	Frightened,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdatedSignature, int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature, EChompGameState, OldState, EChompGameState,
                                             NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChangedSignature, EChompGamePlayingSubstate,
                                             OldSubstate, EChompGamePlayingSubstate, NewSubstate);

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

UCLASS()
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	float ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TArray<FWave> Waves;

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
	int GetNumberOfDotsConsumed() const;
	void LoseGame();
	void StartGame();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	template <typename T>
	struct TFieldWithLastUpdatedTime
	{
	private:
		T Value;
		double LastUpdatedTime;

	public:
		TFieldWithLastUpdatedTime(
			const T Value,
			const UWorld* WorldInstance
		):
			Value(Value),
			LastUpdatedTime(WorldInstance ? WorldInstance->GetRealTimeSeconds() : 0.0)
		{
		}

		TFieldWithLastUpdatedTime& operator=(const TFieldWithLastUpdatedTime& Other)
		{
			if (this == &Other)
				return *this;
			Value = Other.Value;
			LastUpdatedTime = Other.LastUpdatedTime;
			return *this;
		}

		T GetValue() const
		{
			return Value;
		}
	};

	int Score = 0;
	int NumberOfDotsRemaining = 0;
	TFieldWithLastUpdatedTime<int> NumberOfDotsConsumed = TFieldWithLastUpdatedTime(0, nullptr);
	EChompGameState GameState = EChompGameState::None;
	EChompGamePlayingSubstate LastKnownGamePlayingSubstate = EChompGamePlayingSubstate::None;

	// The time (as reported by UWorld::GetTimeSeconds) when the game entered into an EChompGameState::Playing state.
	float GameStartTime = 0.0f;

	void UpdateScore(int NewScore);
	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);
	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);
	float GetTimeSinceStart() const;
	void TransitionTo(EChompGameState NewState);
};
