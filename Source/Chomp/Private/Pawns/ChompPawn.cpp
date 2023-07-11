#include "ChompGameMode.h"
#include "Constants/GameplayTag.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"

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

	//
	// Movement is disabled if the game mode is in a game over state.
	//

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto ChompGameMode = Cast<AChompGameMode>(GameMode);
	check(ChompGameMode);

	if (ChompGameMode->GameState != PacmanGameState::Playing)
		return;

	//
	// Invoke the MoveVector() behavior in the parent class.
	//

	Super::MoveVector(Value, DeltaTime);
}

void AChompPawn::NotifyActorBeginOverlap(AActor *Other)
{

	//
	// Early return if we aren't in a "Playing" state.
	//

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto ChompGameMode = Cast<AChompGameMode>(GameMode);
	check(ChompGameMode);

	if (ChompGameMode->GameState != PacmanGameState::Playing)
		return;

	// If we overlapped with a dot, then consume the other dot.
	if (Other->Tags.Contains(FName(GameplayTag::SmallDot)))
	{
		Other->Destroy();
	}
	// Otherwise, if we overlapped with a ghost, then log for now.
	else if (Cast<AGhostPawn>(Other))
	{
		DEBUG_LOG(TEXT("Overlapped with ghost pawn. Pawn name: %s"), *Other->GetHumanReadableName());
		OnPacmanDiedDelegate.Broadcast();
	}
}

void AChompPawn::NotifyActorEndOverlap(AActor *Other)
{
	// No-op.
}
