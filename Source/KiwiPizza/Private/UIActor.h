#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerationActor.h"
#include "Blueprint/UserWidget.h"
#include "UIActor.generated.h"

UCLASS()
class AUIActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUIActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Level Generator Reference")
	ALevelGenerationActor* LevelGenerator;

	UFUNCTION(BlueprintCallable)
	void HandleDotsCleared();

	UPROPERTY(EditDefaultsOnly, Category = "UWidget References")
	TSubclassOf<UUserWidget> GameOverWinWidgetClass;

private:
	class UGameOverWinWidget* GameOverWinWidgetInstance;

public:
	UFUNCTION()
	void HandleRestartGameClicked();
};
