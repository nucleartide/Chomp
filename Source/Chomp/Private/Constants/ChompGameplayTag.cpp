#include "Constants/ChompGameplayTag.h"

FName FChompGameplayTag::LevelGeometry = FName(TEXT("LevelGeometry"));
FName FChompGameplayTag::SmallDot = FName(TEXT("SmallDot"));
FName FChompGameplayTag::GateTile = FName(TEXT("GateTile"));

bool FChompGameplayTag::ActorHasOneOf(const AActor *SomeActor, TArray<FName> Tags)
{
    for (const auto Tag : Tags)
    {
        if (SomeActor->ActorHasTag(Tag))
        {
            return true;
        }
    }

    return false;
}
