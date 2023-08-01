#include "SafeGet.h"

#include "ChompGameState.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/GhostPawn.h"

class AGhostPawn;

template <class T>
T* FSafeGet::Pawn(const AController* Controller)
{
	auto Pawn = Controller->GetPawn<T>();
	check(Pawn);
	return Pawn;
}

template
AMovablePawn* FSafeGet::Pawn(const AController* Controller);

template
AGhostPawn* FSafeGet::Pawn(const AController* Controller);

template <typename T>
T* FSafeGet::GameState(const AActor* Actor)
{
	const auto WorldInstance = World(Actor);
	auto GameState = WorldInstance->GetGameState<T>();
	check(GameState);
	return GameState;
}

template
AChompGameState* FSafeGet::GameState(const AActor* Actor);

UWorld* FSafeGet::World(const AActor* Actor)
{
	auto World = Actor->GetWorld();
	check(World);
	return World;
}

APlayerController* FSafeGet::PlayerController(const AActor* Actor, int PlayerIndex)
{
	auto WorldInstance = World(Actor);
	auto PlayerController = UGameplayStatics::GetPlayerController(WorldInstance, 0);
	check(PlayerController);
	return PlayerController;
}

// An alternative implementation of FVector::GetSafeNormal that relies on the signs of the values.
// Will throw if the resulting signed vector does not have length 1.
FVector FSafeGet::ActuallyGetSafeNormal(const FVector& Input)
{
	const auto X = FMath::IsNearlyEqual(Input.X, 0.0, 0.01) ? 0.0 : FMath::Sign(Input.X);
	const auto Y = FMath::IsNearlyEqual(Input.Y, 0.0, 0.01) ? 0.0 : FMath::Sign(Input.Y);
	check(
		FMath::IsNearlyEqual(FGenericPlatformMath::Abs(X), 1.0, 0.01) &&
		FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Y), 0.0, 0.01) ||
		FMath::IsNearlyEqual(FGenericPlatformMath::Abs(X), 0.0, 0.01) &&
		FMath::IsNearlyEqual(FGenericPlatformMath::Abs(Y), 1.0, 0.01)
	);
	return FVector{X, Y, 0.0};
}

FVector2D FSafeGet::ActuallyGetSafeNormal(const FVector2D& Input)
{
	const auto Result = ActuallyGetSafeNormal(FVector{Input.X, Input.Y, 0.0});
	return FVector2D{Result.X, Result.Y};
}
