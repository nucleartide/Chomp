#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "ChompGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdatedSignature, int, NewScore);

UCLASS()
class AChompGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void IncrementScore(int increment);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnScoreUpdatedSignature OnScoreUpdatedDelegate;

private:
	int Score = 0;
};
