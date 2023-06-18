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
	virtual void Destroyed() override;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnDotConsumedSignature OnDotConsumedDelegate;
};
