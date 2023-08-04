#pragma once

#include "CoreMinimal.h"
#include "GhostAiController.h"
#include "UObject/Object.h"
#include "Utils/RuntimeSet.h"
#include "GhostHouseQueue.generated.h"

class AGhostAiController;

// AGhostHouseQueue is an implementation of the Runtime Set design pattern,
// as described by Ryan Hipple in his talk on ScriptableObjects in Unity.
UCLASS(Blueprintable)
class AGhostHouseQueue : public ARuntimeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	double TimeToForceGhostOutOfHouse = 5.0;

protected:
	AGhostHouseQueue();
	
	virtual void Tick(float DeltaSeconds) override;

private:
	static bool Sort(const AGhostAiController* A, const AGhostAiController* B)
	{
		return A->GetLeaveGhostHousePriority() > B->GetLeaveGhostHousePriority();
	}
};
