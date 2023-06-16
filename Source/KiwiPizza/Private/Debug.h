#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Debug.generated.h"

#ifdef WITH_EDITOR
    #define DEBUG_LOG(...) UE_LOG(LogTemp, Warning, __VA_ARGS__)
#else
    #define DEBUG_LOG(...)
#endif

UCLASS()
class UDebug : public UObject
{
	GENERATED_BODY()
};
