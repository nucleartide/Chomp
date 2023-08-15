#include "Pawns/ChompPawn.h"

#include "Actors/ConsumableDotActor.h"
#include "Actors/ConsumableEnergizerActor.h"
#include "ChompGameState.h"
#include "Controllers/GhostAiController.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

void AChompPawn::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	if (!ChompGameState->IsPlaying())
		return;

	if (const auto ConsumableDot = Cast<AConsumableDotActor>(Other))
		ConsumableDot->Consume();
	else if (const auto GhostPawn = Cast<AGhostPawn>(Other))
	{
		const auto GhostController = GhostPawn->GetController<AGhostAiController>();
		if (const auto HasBeenEaten = GhostController->GetHasBeenEaten(); !HasBeenEaten)
		{
			ChompGameState->LoseGame();
			Destroy();
		}
	}
	else if (const auto EnergizerDot = Cast<AConsumableEnergizerActor>(Other))
		EnergizerDot->Consume();
}
