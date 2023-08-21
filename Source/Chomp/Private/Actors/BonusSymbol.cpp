#include "BonusSymbol.h"

void ABonusSymbol::Consume()
{
    // TODO: GetWorld()->GetGameState<AChompGameState>()->ConsumeDot();
    Destroy();
}
