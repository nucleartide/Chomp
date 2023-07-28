#pragma once

#include "CoreMinimal.h"

class FChompGameplayTag
{
public:
	static FName LevelGeometry;
	static FName SmallDot;
	static FName GateTile;
	static bool ActorHasOneOf(const AActor *SomeActor, TArray<FName> Tags);
};
