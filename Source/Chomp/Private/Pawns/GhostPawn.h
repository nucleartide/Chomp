#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "Pawns/MovablePawn.h"
#include "AStar/GridLocation.h"
#include "GhostPawn.generated.h"

class AGhostHouseQueue;

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

	// Higher number means higher priority. Similar to z-index in CSS.
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	int LeaveGhostHousePriority = 0;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	AGhostHouseQueue* GhostHouseQueue;

public:
	FGridLocation GetStartingPosition() const;
	int GetDotsConsumedMovementThreshold() const;
	FGridLocation GetScatterOrigin() const;
	FGridLocation GetScatterDestination() const;
	int GetLeaveGhostHousePriority() const;
	AGhostHouseQueue* GetGhostHouseQueue() const;

protected:
	virtual void BeginPlay() override;
};
