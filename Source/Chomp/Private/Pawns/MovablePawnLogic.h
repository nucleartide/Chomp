#pragma once

#include "CoreMinimal.h"
#include "MovablePawn.h"
#include "MovablePawnLogic.generated.h"

struct FMoveTowardsPointResult
{
	bool IsValid;
	int Thing;
};

UCLASS()
class CHOMP_API UMovablePawnLogic : public UObject
{
	GENERATED_BODY()
	
public:
	explicit UMovablePawnLogic();
	explicit UMovablePawnLogic(ULevelLoader *LevelInstance);
	static FMoveTowardsPointResult MoveTowardsPoint(
		const FGridLocation& TargetGridPosition,
		const FGridLocation& TargetDirection,
		float DeltaTime,
		FVector Location,
		FRotator Rotation);

private:
	UPROPERTY()
	TObjectPtr<ULevelLoader> LevelInstance;
};
