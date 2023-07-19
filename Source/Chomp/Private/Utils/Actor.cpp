#include "Utils/Actor.h"

FVector2D GetActorLocation2D(AActor *SomeActor)
{
    auto ActorLocation = SomeActor->GetActorLocation();
    FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
    return ActorLocation2D;
}
