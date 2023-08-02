#include "Pawns/ChompPawn.h"

#include "ChompGameState.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Actors/ConsumableDotActor.h"

void AChompPawn::NotifyActorBeginOverlap(AActor *Other)
{
	if (GetWorld()->GetGameState<AChompGameState>()->GetEnum() != EChompGameState::Playing)
		return;

	if (const auto PossibleDot = Cast<AConsumableDotActor>(Other))
	{
		PossibleDot->Consume();
	}
	else if (Cast<AGhostPawn>(Other))
	{
		DEBUG_LOG(TEXT("Overlapped with ghost pawn. Pawn name: %s"), *Other->GetHumanReadableName());
		GetWorld()->GetGameState<AChompGameState>()->LoseGame();
		Destroy(); // TODO. investigate this behavior.
	}
}
