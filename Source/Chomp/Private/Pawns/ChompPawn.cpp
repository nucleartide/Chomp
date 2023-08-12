#include "Pawns/ChompPawn.h"

#include "Actors/ConsumableDotActor.h"
#include "Actors/ConsumableEnergizerActor.h"
#include "ChompGameState.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

void AChompPawn::NotifyActorBeginOverlap(AActor* Other)
{
	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	if (ChompGameState->GetEnum() != EChompGameStateEnum::Playing)
		return;

	if (const auto ConsumableDot = Cast<AConsumableDotActor>(Other))
	{
		ConsumableDot->Consume();
	}
	else if (Cast<AGhostPawn>(Other))
	{
		DEBUG_LOG(TEXT("Overlapped with ghost pawn. Pawn name: %s"), *Other->GetHumanReadableName());
		ChompGameState->LoseGame();
		Destroy();
	}
	else if (const auto EnergizerDot = Cast<AConsumableEnergizerActor>(Other))
	{
		EnergizerDot->Consume();
	}
}
