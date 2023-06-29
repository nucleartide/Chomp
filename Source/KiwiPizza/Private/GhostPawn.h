#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "MovablePawn.h"
#include "GhostPawn.generated.h"

UCLASS()
class AGhostPawn : public AMovablePawn
{
	GENERATED_BODY()

public:
	AGhostPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	class UStaticMeshComponent* HeadStaticMesh;
	class UStaticMeshComponent* BodyStaticMesh;
};
