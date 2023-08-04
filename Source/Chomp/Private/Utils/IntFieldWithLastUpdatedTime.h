#pragma once

#include "CoreMinimal.h"
#include "IntFieldWithLastUpdatedTime.generated.h"

USTRUCT(BlueprintType)
struct FIntFieldWithLastUpdatedTime
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	int Value;

	UPROPERTY(VisibleAnywhere)
	double LastUpdatedTime;

public:
	FIntFieldWithLastUpdatedTime(): Value(0), LastUpdatedTime(0.0)
	{
	}

	FIntFieldWithLastUpdatedTime(
		const int Value,
		const UWorld* WorldInstance
	):
		Value(Value),
		LastUpdatedTime(WorldInstance ? WorldInstance->GetRealTimeSeconds() : 0.0)
	{
	}

	FIntFieldWithLastUpdatedTime& operator=(const FIntFieldWithLastUpdatedTime& Other)
	{
		if (this == &Other)
			return *this;
		Value = Other.Value;
		LastUpdatedTime = Other.LastUpdatedTime;
		return *this;
	}

	int GetValue() const
	{
		return Value;
	}

	double GetLastUpdatedTime() const
	{
		return LastUpdatedTime;
	}
};
