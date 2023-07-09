#include "GhostPawn.h"
#include "Utils/Debug.h"

AGhostPawn::AGhostPawn()
{
	PrimaryActorTick.bCanEverTick = true;
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

void AGhostPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGhostPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
