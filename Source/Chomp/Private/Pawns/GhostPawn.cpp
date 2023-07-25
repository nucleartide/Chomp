#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"

AGhostPawn::AGhostPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentScatterOrigin = ScatterOrigin;
	CurrentScatterDestination = ScatterDestination;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	auto HeadComponents = GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("Head"));
	check(HeadComponents.Num() == 1);

	auto BodyComponents = GetComponentsByTag(UStaticMeshComponent::StaticClass(), FName("Body"));
	check(BodyComponents.Num() == 1);

	DEBUG_LOG(TEXT("Head component name: %s"), *HeadComponents[0]->GetReadableName());
	DEBUG_LOG(TEXT("Body component name: %s"), *BodyComponents[0]->GetReadableName());
}

FGridLocation AGhostPawn::GetStartingPosition() const
{
	return StartingPosition;
}

int AGhostPawn::GetDotsConsumedMovementThreshold() const
{
	return DotsConsumedMovementThreshold;
}

FGridLocation AGhostPawn::GetScatterDestination() const
{
	return ScatterDestination;
}

void AGhostPawn::SwapScatterOriginAndDestination()
{
	const FGridLocation Swap{CurrentScatterOrigin.X, CurrentScatterOrigin.Y};
	CurrentScatterOrigin = CurrentScatterDestination;
	CurrentScatterDestination = Swap;
}
