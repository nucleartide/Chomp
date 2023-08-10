#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/ChompGameStateEnum.h"
#include "GameState/ChompPlayingSubstateEnum.h"
#include "GameState/CurrentSubstate.h"
#include "Utils/IntFieldWithLastUpdatedTime.h"
#include "ChompGameState.generated.h"

struct FWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsClearedSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature,
                                            int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdatedSignature,
                                            int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedSignature,
                                             EChompGameStateEnum, OldState,
                                             EChompGameStateEnum, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChangedSignature,
                                             EChompPlayingSubstateEnum, OldSubstate,
                                             EChompPlayingSubstateEnum, NewSubstate);

UCLASS(Blueprintable)
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly)
	TArray<FWave> Waves;

	UPROPERTY(EditDefaultsOnly)
	double FrightenedSubstateDuration = 5.0;

	UPROPERTY(VisibleInstanceOnly)
	int Score = 0;

	UPROPERTY(VisibleInstanceOnly)
	int NumberOfDotsRemaining = 0;

	UPROPERTY(VisibleInstanceOnly)
	FIntFieldWithLastUpdatedTime NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime(0, nullptr);

	UPROPERTY(VisibleInstanceOnly)
	EChompGameStateEnum GameState = EChompGameStateEnum::None;

	UPROPERTY(VisibleInstanceOnly)
	FCurrentSubstate CurrentSubstate = FCurrentSubstate(Waves, FrightenedSubstateDuration);

	void UpdateScore(int NewScore);

	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);

	void TransitionTo(EChompGameStateEnum NewState);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

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

	void ConsumeEnergizerDot();

	EChompGameStateEnum GetEnum() const;

	int GetScore() const;

	FIntFieldWithLastUpdatedTime GetNumberOfDotsConsumed() const;

	void LoseGame();

	void StartGame();

	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);

	EChompPlayingSubstateEnum GetSubstateEnum() const;
};
