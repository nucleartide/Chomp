#pragma once

#include "CoreMinimal.h"
#include "ChompGameState.h"
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
	/**
	 * A reference to the map's LevelGenerationActor, so that we can listen for level cleared events.
	 */
	UPROPERTY(EditAnywhere, Category = "Custom Settings")
	ALevelGenerationActor* LevelGenerator;

	/**
	 * The UI to display when the player wins the game.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<UUserWidget> GameOverWinWidgetClass;

	/**
	 * The UI to display when the player loses the game.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Settings")
	TSubclassOf<UUserWidget> GameOverLoseWidgetClass;

	AUIManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	class UGameOverWidget* GameOverWidgetInstance;

	UFUNCTION()
	void HandleDotsCleared();

	UFUNCTION()
	void HandlePlayerDeath(EChompGameState OldState, EChompGameState NewState);

	UFUNCTION()
	void HandleRestartGameClicked();
};
