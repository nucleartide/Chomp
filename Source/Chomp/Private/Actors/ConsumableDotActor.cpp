#include "Actors/ConsumableDotActor.h"

void AConsumableDotActor::Consume()
{
    OnDotConsumedDelegate.Broadcast();
    Destroy();
}
