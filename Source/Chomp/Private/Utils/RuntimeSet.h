#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <functional>
#include "RuntimeSet.generated.h"

// ARuntimeSet is an implementation of the Runtime Set design pattern,
// as described by Ryan Hipple in his talk on ScriptableObjects in Unity.
UCLASS(Blueprintable)
class CHOMP_API ARuntimeSet : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> Things;
	
public:
	void AddAndSort(AActor* Thing, bool(*Comparator)(AActor*, AActor*))
	{
		Things.Add(Thing);
		Algo::Sort(Things, Comparator);
	}

	void Remove(AActor* Thing)
	{
		Things.Remove(Thing);
	}

	bool Contains(const AActor* Thing) const
	{
		return Things.Contains(Thing);
	}
};
