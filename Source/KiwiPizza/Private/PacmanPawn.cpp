#include "PacmanPawn.h"
#include "PacmanGameMode.h"
#include "Debug.h"
#include "GhostPawn.h"

APacmanPawn::APacmanPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APacmanPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	WrapAroundWorld();
}

void APacmanPawn::MoveVector(FVector2D Value)
{

	//
	// Movement is disabled if the game mode is in a game over state.
	//

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	if (PacmanGameMode->GameState != PacmanGameState::Playing)
		return;

	//
	// Invoke the MoveVector() behavior in the parent class.
	//

	Super::MoveVector(Value);
}

void APacmanPawn::NotifyActorBeginOverlap(AActor *Other)
{

	//
	// Early return if we aren't in a "Playing" state.
	//

	auto GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);

	auto PacmanGameMode = Cast<APacmanGameMode>(GameMode);
	check(PacmanGameMode);

	if (PacmanGameMode->GameState != PacmanGameState::Playing)
		return;

	// If we overlapped with a dot, then consume the other dot.
	if (Other->Tags.Contains(FName("SmallDot"))) // TODO: One could check tags or cast against a type. Checking tags is prone to typos, but casting has dependency issues. Wonder which is the better tradeoff?
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

void APacmanPawn::NotifyActorEndOverlap(AActor *Other)
{
	// No-op.
}
