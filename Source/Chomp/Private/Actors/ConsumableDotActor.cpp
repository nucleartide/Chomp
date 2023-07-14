#include "Actors/ConsumableDotActor.h"
#include "ChompGameState.h"

void AConsumableDotActor::Consume()
{
    GetWorld()->GetGameState<AChompGameState>()->ConsumeDot();
    Destroy();
}
