#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "UObject/Object.h"
#include "AGhostHouseQueue.generated.h"

class AGhostAiController;

// AGhostHouseQueue is an implementation of the Runtime Set design pattern,
// as described by Ryan Hipple in his talk on ScriptableObjects in Unity.
UCLASS(Blueprintable)
class AGhostHouseQueue : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<AGhostAiController*> Things;
	
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	double TimeToForceGhostOutOfHouse = 5.0;

public:
	void AddAndSort(AGhostAiController* Thing)
	{
		Things.Add(Thing);
		Things.Sort(&AGhostHouseQueue::Sort);
	}

	void Remove(AGhostAiController* Thing)
	{
		Things.Remove(Thing);
	}

	bool IsOnTop(const AGhostAiController* Thing) const
	{
		return Things.Top() == Thing;
	}

	bool Contains(const AGhostAiController* Thing) const
	{
		return Things.Contains(Thing);
	}

protected:
	AGhostHouseQueue();
	virtual void Tick(float DeltaSeconds) override;

private:
	static bool Sort(const AGhostAiController& A, const AGhostAiController& B)
	{
		return A.GetLeaveGhostHousePriority() > B.GetLeaveGhostHousePriority();
	}
	
	UFUNCTION()
	void HandleDotsConsumedUpdated(int NewDotsConsumed);
};
