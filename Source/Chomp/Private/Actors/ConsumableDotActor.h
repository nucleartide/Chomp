#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ConsumableDotActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDotConsumedSignature);

UCLASS()
class AConsumableDotActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnDotConsumedSignature OnDotConsumedDelegate;

	/**
	 * Consume the dot.
	 *
	 * We need this function to distinguish between consuming a dot (and destroying it), versus destroying it as part of a level reset.
	 */
	UFUNCTION()
	void Consume();
};
