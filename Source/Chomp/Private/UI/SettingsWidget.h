#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

UCLASS()
class CHOMP_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	// UButton* QuitButton;
	
	virtual void NativeConstruct() override;
};
