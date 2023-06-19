#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "GhostPawn.generated.h"

UCLASS()
class AGhostPawn : public APawn
{
	GENERATED_BODY()

public:
	AGhostPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "Component References")
	class UStaticMeshComponent* HeadStaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Component References")
	class UStaticMeshComponent* BodyStaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Level Loader Customization")
	FLinearColor Color;

	// color to set the ghost to
	// on beginplay, both meshes get set to the specified color
};
