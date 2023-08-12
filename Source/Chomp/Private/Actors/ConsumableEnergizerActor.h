#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "ConsumableEnergizerActor.generated.h"

UCLASS()
class CHOMP_API AConsumableEnergizerActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	/**
	 * Consume the dot.
	 *
	 * We need this function to distinguish between consuming a dot (and destroying it), versus destroying it as part of a level reset.
	 */
	UFUNCTION()
	void Consume();
};
