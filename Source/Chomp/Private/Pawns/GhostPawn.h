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

public:
	AGhostPawn();
	FGridLocation GetStartingPosition();
	int GetDotsConsumedMovementThreshold() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	class UStaticMeshComponent* HeadStaticMesh;
	class UStaticMeshComponent* BodyStaticMesh;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	FGridLocation StartingPosition;

	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	int DotsConsumedMovementThreshold = 0;
};
