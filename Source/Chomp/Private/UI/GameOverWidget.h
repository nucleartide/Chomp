#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartGameClickedSignature);

UCLASS(BlueprintType, Blueprintable)
class UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton *RestartGameButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRestartGame() const;

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRestartGameClickedSignature OnRestartGameClickedDelegate;
};
