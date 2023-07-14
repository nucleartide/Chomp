#include "Pawns/ChompPawn.h"

#include "ChompGameMode.h"
#include "ChompGameState.h"
#include "Constants/GameplayTag.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Actors/ConsumableDotActor.h"

AChompPawn::AChompPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChompPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AChompPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WrapAroundWorld();
}

void AChompPawn::MoveVector(FVector2D Value, float DeltaTime)
{
	if (GetWorld()->GetGameState<AChompGameState>()->GetEnum() != EChompGameState::Playing)
		return;

	Super::MoveVector(Value, DeltaTime);
}

void AChompPawn::NotifyActorBeginOverlap(AActor *Other)
{
	if (GetWorld()->GetGameState<AChompGameState>()->GetEnum() != EChompGameState::Playing)
		return;

	// If we overlapped with a dot, then consume the other dot.
	auto PossibleDot = Cast<AConsumableDotActor>(Other);
	if (PossibleDot)
	{
		PossibleDot->Consume();
	}
	// Otherwise, if we overlapped with a ghost, then log for now.
	else if (Cast<AGhostPawn>(Other))
	{
		DEBUG_LOG(TEXT("Overlapped with ghost pawn. Pawn name: %s"), *Other->GetHumanReadableName());
		GetWorld()->GetGameState<AChompGameState>()->NotifyPlayerDeath();
		Destroy();
	}
}

void AChompPawn::NotifyActorEndOverlap(AActor *Other)
{
	// No-op.
}
