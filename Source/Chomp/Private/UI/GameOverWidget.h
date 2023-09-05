#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRestartGameClickedSignature);

UCLASS(BlueprintType, Blueprintable)
class UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// TODO: You should call this in a Blueprint subclass.
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRestartGameClickedSignature OnRestartGameClickedDelegate;
};
