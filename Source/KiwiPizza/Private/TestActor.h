#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelLoader.h"
#include "TestActor.generated.h"

/**
 * ATestActor is an actor that is for trying pieces of code.
 */
UCLASS()
class ATestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATestActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Level Loader Customization")
	TSubclassOf<ULevelLoader> Level;
};
