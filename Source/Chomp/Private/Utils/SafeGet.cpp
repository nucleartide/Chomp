#include "SafeGet.h"

#include "ChompGameState.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/GhostPawn.h"

class AGhostPawn;

template <class T>
T* FSafeGet::Pawn(AController* Controller)
{
	auto Pawn = Controller->GetPawn<T>();
	check(Pawn);
	return Pawn;
}

template
AMovablePawn* FSafeGet::Pawn(AController* Controller);

template
AGhostPawn* FSafeGet::Pawn(AController* Controller);

template <typename T>
T* FSafeGet::GameState(AActor* Actor)
{
	const auto WorldInstance = World(Actor);
	auto GameState = WorldInstance->GetGameState<T>();
	check(GameState);
	return GameState;
}

template
AChompGameState* FSafeGet::GameState(AActor* Actor);

UWorld* FSafeGet::World(AActor* Actor)
{
	auto World = Actor->GetWorld();
	check(World);
	return World;
}

APlayerController* FSafeGet::PlayerController(AActor* Actor, int PlayerIndex)
{
	auto WorldInstance = World(Actor);
	auto PlayerController = UGameplayStatics::GetPlayerController(WorldInstance, 0);
	check(PlayerController);
	return PlayerController;
}
