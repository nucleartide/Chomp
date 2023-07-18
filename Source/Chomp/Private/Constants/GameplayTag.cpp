#include "Constants/GameplayTag.h"

FName GameplayTag::LevelGeometry = FName(TEXT("LevelGeometry"));
FName GameplayTag::SmallDot = FName(TEXT("SmallDot"));
FName GameplayTag::GateTile = FName(TEXT("GateTile"));

bool GameplayTag::ActorHasOneOf(AActor *SomeActor, TArray<FName> Tags)
{
    for (auto Tag : Tags)
    {
        if (SomeActor->ActorHasTag(Tag))
        {
            return true;
        }
    }

    return false;
}
