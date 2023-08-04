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

public:
	void Add(const AActor* Thing)
	{
		AddAndSort(Thing, &SortByGhostHousePriority);
	}

protected:
	AGhostHouseQueue();
	
	virtual void Tick(float DeltaSeconds) override;

private:
	static bool SortByGhostHousePriority(const AActor* A, const AActor* B)
	{
		const auto GhostA = Cast<AGhostAiController>(A);
		check(GhostA);
		const auto GhostB = Cast<AGhostAiController>(B);
		check(GhostB);

		// Array will be sorted in ascending order: 1, 10, 100, 1000, 10000
		return GhostA->GetLeaveGhostHousePriority() < GhostB->GetLeaveGhostHousePriority();
	}
};
