#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LivesWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ULivesWidget : public UUserWidget
{
	GENERATED_BODY()

#if false
protected:
	uproperty(meta = (BindWidget))
	class UButton *RestartGameButton;

	virtual void NativeConstruct() override;

	ufunction()
	void HandleRestartGame();
#endif
};
