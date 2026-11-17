#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMossUnreal, Log, All);

/** Consistent logging entry points for game code and the integration itself. */
struct MOSSUNREAL_API FMossDiagnostics final
{
    static void Log(const FString& Message);
    static void Warning(const FString& Message);
    static void Error(const FString& Message);
};
