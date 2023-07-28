#include "Utils/Actor.h"

FVector2D GetActorLocation2D(const AActor *SomeActor)
{
    const auto ActorLocation = SomeActor->GetActorLocation();
    const FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
    return ActorLocation2D;
}
