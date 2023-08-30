#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UButton;

UCLASS(BlueprintType, Blueprintable)
class CHOMP_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BackButton;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* ApplyButton;
	
	virtual void NativeConstruct() override;
};
