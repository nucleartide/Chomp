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

private:
	/**
	 * Called when the actor has been explicitly destroyed.
	 */
	virtual void Destroyed() override;
};
