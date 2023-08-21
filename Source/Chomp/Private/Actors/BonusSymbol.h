#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BonusSymbol.generated.h"

UCLASS()
class CHOMP_API ABonusSymbol : public AStaticMeshActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int PointsToAward = 1000;

public:
	void Consume();
};
