#include "BonusSymbol.h"

#include "ChompGameState.h"
#include "Utils/SafeGet.h"

void ABonusSymbol::Consume()
{
    const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
    const auto CurrentScore = ChompGameState->GetScore();
    ChompGameState->UpdateScore(CurrentScore + PointsToAward);
    Destroy();
}
