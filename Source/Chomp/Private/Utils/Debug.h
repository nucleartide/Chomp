#pragma once

#include "CoreMinimal.h"
#include "Debug.generated.h"

// Place this in a header file and include it to reuse a log category between multiple source files
// Arguments:
// 1. Name of your custom category. You can use LogTemp if you don't want to define a category.
// 2. Default verbosity when one is not specified. The most common value is Log.
// Valid verbosity levels are: Fatal, Error, Warning, Display, Log, Verbose, VeryVerbose
// 3. Maximum verbosity level to allow when compiling. Can also be All
DECLARE_LOG_CATEGORY_EXTERN(GridAlignedMovement, Log, All);

#ifdef WITH_EDITOR
    #define DEBUG_LOG(...) UE_LOG(LogTemp, Warning, __VA_ARGS__)
#else
    #define DEBUG_LOG(...)
#endif

#ifdef WITH_EDITOR
    #define DEBUG_LOGERROR(...) UE_LOG(LogTemp, Error, __VA_ARGS__)
#else
    #define DEBUG_LOGERROR(...)
#endif

#ifdef WITH_EDITOR
    #define DEBUG_MOVE(...) UE_LOG(GridAlignedMovement, Warning, __VA_ARGS__)
#else
    #define DEBUG_LOG(...)
#endif

UCLASS()
class UDebug : public UObject
{
	GENERATED_BODY()
};
