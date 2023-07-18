#pragma once

#include "CoreMinimal.h"

class GameplayTag
{
public:
	static FName LevelGeometry;
	static FName SmallDot;
	static FName GateTile;

public:
	static bool ActorHasOneOf(AActor *SomeActor, TArray<FName> Tags);
};
