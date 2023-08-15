#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/ChompGameStateEnum.h"
#include "GameState/ChompPlayingSubstateEnum.h"
#include "GameState/CurrentSubstate.h"
#include "Utils/IntFieldWithLastUpdatedTime.h"
#include "ChompGameState.generated.h"

struct FWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsCleared);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated,
                                            int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdated,
                                            int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChanged,
                                             EChompGameStateEnum, OldState,
                                             EChompGameStateEnum, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePlayingStateChanged,
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
	FOnDotsCleared OnDotsClearedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsConsumedUpdated OnDotsConsumedUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChanged OnGameStateChangedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnScoreUpdated OnScoreUpdatedDelegate;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGamePlayingStateChanged OnGamePlayingStateChangedDelegate;

	AChompGameState();

	void ResetDots(int NumberOfDots);

	void ConsumeDot();

	void ConsumeEnergizerDot();

	EChompGameStateEnum GetEnum() const;

	bool IsPlaying() const;

	bool IsFrightened() const;

	int GetScore() const;

	FIntFieldWithLastUpdatedTime GetNumberOfDotsConsumed() const;

	void LoseGame();

	void StartGame();

	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);

	EChompPlayingSubstateEnum GetSubstateEnum(const bool ExcludeFrightened = false) const;
};
