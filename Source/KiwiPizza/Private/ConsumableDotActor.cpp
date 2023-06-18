#include "ConsumableDotActor.h"

void AConsumableDotActor::Destroyed()
{
    Super::Destroyed();
    OnDotConsumedDelegate.Broadcast();
}
