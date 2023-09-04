#pragma once

#include "CoreMinimal.h"
#include "UE5Coro.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/ChompGameStateEnum.h"
#include "GameState/ChompPlayingSubstate.h"
#include "GameState/ChompPlayingSubstateEnum.h"
#include "Utils/IntFieldWithLastUpdatedTime.h"
#include "ChompGameState.generated.h"

class ULevelSymbolWidget;
class ULevelDataAsset;
class ULevelSequenceDataAsset;
struct FWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotsCleared);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOneUp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated,
                                            int, Score);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDotsConsumedUpdated,
                                            int, NewDotsConsumed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged,
                                            int, NewNumOfLives);

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

	// Amount to multiply the score by.
	// Example: consuming a dot is 1 point multiplied by ScoreMultiplier, giving 10 points as a result.
	UPROPERTY(EditDefaultsOnly)
	int ScoreMultiplier = 10;

	UPROPERTY(EditDefaultsOnly)
	int OneUpThreshold = 2000;

	UPROPERTY(EditDefaultsOnly)
	TArray<FWave> Waves;

	UPROPERTY(EditDefaultsOnly)
	double FrightenedSubstateDuration = 5.0;

	UPROPERTY(VisibleInstanceOnly)
	int Score = 0;

	UPROPERTY(VisibleInstanceOnly)
	int OneUpCounter = 0;

	UPROPERTY(VisibleInstanceOnly)
	int NumberOfDotsRemaining = 0;

	UPROPERTY(VisibleInstanceOnly)
	int NumberOfLives = 0;

	UPROPERTY(EditDefaultsOnly)
	int StartingNumberOfLives = 3;

	UPROPERTY(VisibleInstanceOnly)
	FIntFieldWithLastUpdatedTime NumberOfDotsConsumed = FIntFieldWithLastUpdatedTime();

	UPROPERTY(VisibleInstanceOnly)
	EChompGameStateEnum GameState = EChompGameStateEnum::None;

	UPROPERTY(VisibleInstanceOnly)
	FChompPlayingSubstate CurrentSubstate = FChompPlayingSubstate(FrightenedSubstateDuration, Waves);

	UPROPERTY(VisibleInstanceOnly)
	EChompPlayingSubstateEnum LastSubstateEnum = EChompPlayingSubstateEnum::None;

	UPROPERTY(EditDefaultsOnly, Category = "Hardcoded Current Level")
	ULevelDataAsset* CurrentLevel;

	void UpdateNumberOfDotsRemaining(int NewNumberOfDotsRemaining);

	void TransitionTo(EChompGameStateEnum NewState);
	
	void UpdateNumberOfLives(int NewNumOfLives);
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsCleared OnDotsCleared;
	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnOneUp OnOneUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnDotsConsumedUpdated OnDotsConsumedUpdated;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGameStateChanged OnGameStateChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnScoreUpdated OnScoreUpdated;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnGamePlayingStateChanged OnGamePlayingStateChanged;
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnLivesChanged OnLivesChanged;

	AChompGameState();

	void ResetDots(int NumberOfDots);

	void ConsumeDot();

	void ConsumeEnergizerDot();

	void ConsumeGhost();

	EChompGameStateEnum GetEnum() const;

	bool IsPlaying() const;

	bool IsFrightened() const;

	int GetScore() const;

	void UpdateScore(int NewScore);

	FIntFieldWithLastUpdatedTime GetNumberOfDotsConsumed() const;

	void StartGame();

	void UpdateNumberOfDotsConsumed(const int NewNumberOfDotsConsumed);

	EChompPlayingSubstateEnum GetSubstateEnum(const bool GetUnderlyingSubstate = false) const;

	UE5Coro::TCoroutine<> LoseLife();
};
