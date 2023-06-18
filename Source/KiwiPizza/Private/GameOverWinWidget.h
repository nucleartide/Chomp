#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWinWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartGameClickedSignature);

UCLASS(BlueprintType, Blueprintable)
class UGameOverWinWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton *RestartGameButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRestartGame();

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRestartGameClickedSignature OnRestartGameClickedDelegate;
};
