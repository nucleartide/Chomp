#include "TestActor.h"
#include "Utils/Debug.h"
#include "ChompGameMode.h"

ATestActor::ATestActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	{
		Level.GetDefaultObject()->LoadLevel();
		DEBUG_LOG(TEXT("Level width: %d"), Level.GetDefaultObject()->GetLevelWidth());
		DEBUG_LOG(TEXT("Level height: %d"), Level.GetDefaultObject()->GetLevelHeight());
	}

	{
		DEBUG_LOG(TEXT("Level width: %d"), Level.GetDefaultObject()->GetLevelWidth());
		DEBUG_LOG(TEXT("Level height: %d"), Level.GetDefaultObject()->GetLevelHeight());
	}
}

void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
