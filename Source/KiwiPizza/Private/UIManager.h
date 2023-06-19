#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerationActor.h"
#include "Blueprint/UserWidget.h"
#include "UIManager.generated.h"

UCLASS()
class AUIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AUIManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Level Generator Reference")
	ALevelGenerationActor* LevelGenerator;

	UFUNCTION(BlueprintCallable)
	void HandleDotsCleared();

	UPROPERTY(EditDefaultsOnly, Category = "UWidget References")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

private:
	class UGameOverWidget* GameOverWidgetInstance;

public:
	UFUNCTION()
	void HandleRestartGameClicked();
};
