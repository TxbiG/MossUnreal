#pragma once

#include "CoreMinimal.h"

/** General utility functions available from ordinary C++ without an Actor/Component. */
struct MOSSUNREAL_API FMossUtilities final
{
    static FString MakeUniqueId();
    static double GetMonotonicSeconds();
    static float ClampDeltaSeconds(float DeltaSeconds, float Maximum = 0.25f);
    static bool IsValidIdentifier(const FString& Identifier);
};
