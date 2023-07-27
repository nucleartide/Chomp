#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "Pawns/MovablePawn.h"
#include "AStar/GridLocation.h"
#include "GhostPawn.generated.h"

UCLASS()
class AGhostPawn : public AMovablePawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	FGridLocation StartingPosition;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	int DotsConsumedMovementThreshold = 0;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	FGridLocation ScatterOrigin;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	FGridLocation ScatterDestination;

public:
	AGhostPawn();
	FGridLocation GetStartingPosition() const;
	int GetDotsConsumedMovementThreshold() const;
	FGridLocation GetScatterOrigin() const;
	FGridLocation GetScatterDestination() const;

protected:
	virtual void BeginPlay() override;
};
