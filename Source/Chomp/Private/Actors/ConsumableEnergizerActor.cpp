#include "ConsumableEnergizerActor.h"
#include "ChompGameState.h"

void AConsumableEnergizerActor::Consume()
{
	GetWorld()->GetGameState<AChompGameState>()->ConsumeEnergizerDot();
	Destroy();
}
