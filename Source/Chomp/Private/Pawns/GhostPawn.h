#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "Pawns/MovablePawn.h"
#include "AStar/GridLocation.h"
#include "GhostPawn.generated.h"

enum class EGhostState : uint8;
enum class EChompPlayingSubstateEnum : uint8;
class AGhostHouseQueue;

UCLASS()
class AGhostPawn : public AMovablePawn
{
	GENERATED_BODY()

	// Material that is displayed when ghost is not in a frightened state.
	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	UMaterialInterface* NotFrightenedMaterial;

	// Material that is displayed when ghost is in a frightened state.
	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	UMaterialInterface* FrightenedMaterial;

	UPROPERTY(VisibleInstanceOnly, Category = "Custom Settings")
	UStaticMeshComponent* HeadComponentRef;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Custom Settings")
	UStaticMeshComponent* BodyComponentRef;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	FGridLocation StartingPosition;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	int DotsConsumedMovementThreshold = 0;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	FGridLocation ScatterOrigin;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	FGridLocation ScatterDestination;

	// Higher number means higher priority. Similar to z-index in CSS.
	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	int LeaveGhostHousePriority = 0;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	AGhostHouseQueue* GhostHouseQueue;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	FLinearColor DebugColor = FLinearColor::Red;
	
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	double FrightenedMovementSpeed = 2.5;

	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	double ReturnToGhostHouseMovementSpeed = 0.0;

	UPROPERTY(EditInstanceOnly, Category = "Custom Settings")
	FGridLocation GhostHouseReturnLocation;

	UFUNCTION()
	void UpdateVisibility(const EGhostState NewGhostState);

	UFUNCTION()
	void UpdateMaterial(const EGhostState NewGhostState);

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void NotifyActorBeginOverlap(AActor* Other) override;

public:
	FGridLocation GetStartingPosition() const;
	
	FGridLocation GetGhostHouseReturnLocation() const;
	
	int GetDotsConsumedMovementThreshold() const;
	
	FGridLocation GetScatterOrigin() const;
	
	FGridLocation GetScatterDestination() const;
	
	int GetLeaveGhostHousePriority() const;
	
	AGhostHouseQueue* GetGhostHouseQueue() const;
	
	FLinearColor GetDebugColor() const;

	double GetFrightenedMovementSpeed() const;

	double GetReturnToGhostHouseMovementSpeed() const;
};
