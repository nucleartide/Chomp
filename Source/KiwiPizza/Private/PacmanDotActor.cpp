#include "PacmanDotActor.h"

void APacmanDotActor::Destroyed()
{
    Super::Destroyed();
    OnDotConsumedDelegate.Broadcast();
}
