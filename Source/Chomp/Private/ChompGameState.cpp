#include "ChompGameState.h"

void AChompGameState::IncrementScore(int Increment)
{
    Score += Increment * 10;
    OnScoreUpdatedDelegate.Broadcast(Score);
}
