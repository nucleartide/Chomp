#include "GhostPawn.h"

AGhostPawn::AGhostPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AGhostPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGhostPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
