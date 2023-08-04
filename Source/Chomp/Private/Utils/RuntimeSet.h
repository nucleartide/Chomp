#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeSet.generated.h"

// ARuntimeSet is an implementation of the Runtime Set design pattern,
// as described by Ryan Hipple in his talk on ScriptableObjects in Unity.
UCLASS(Blueprintable)
class CHOMP_API ARuntimeSet : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleInstanceOnly)
	TArray<const AActor*> Things;
	
public:
	void AddAndSort(const AActor* Thing, bool(*Comparator)(const AActor*, const AActor*))
	{
		Things.Add(Thing);
		Algo::Sort(Things, Comparator);
	}

	void Remove(const AActor* Thing)
	{
		Things.Remove(Thing);
	}

	bool Contains(const AActor* Thing) const
	{
		return Things.Contains(Thing);
	}
};
