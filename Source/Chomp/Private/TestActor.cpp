#include "TestActor.h"
#include "Utils/Debug.h"

ATestActor::ATestActor(): AActor()
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

void ATestActor::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}
