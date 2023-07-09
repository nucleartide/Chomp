#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator/LevelGenerationActor.h"
#include "Blueprint/UserWidget.h"
#include "UIManager.generated.h"

/**
 * AUIManager manages the lifetime of the user interface of the game.
 */
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

	UPROPERTY(EditAnywhere, Category = "Level Generator Reference")
	class APacmanPawn* PacmanPawn;

	UFUNCTION(BlueprintCallable)
	void HandleDotsCleared();

	UFUNCTION(BlueprintCallable)
	void HandlePlayerDeath();

	UPROPERTY(EditDefaultsOnly, Category = "UWidget References")
	TSubclassOf<UUserWidget> GameOverWinWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UWidget References")
	TSubclassOf<UUserWidget> GameOverLoseWidgetClass;

private:
	class UGameOverWidget* GameOverWidgetInstance;

public:
	UFUNCTION()
	void HandleRestartGameClicked();
};
