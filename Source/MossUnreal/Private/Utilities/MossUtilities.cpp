#include "Utilities/MossUtilities.h"

#include "HAL/PlatformTime.h"

FString FMossUtilities::MakeUniqueId()
{
    return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

double FMossUtilities::GetMonotonicSeconds()
{
    return FPlatformTime::Seconds();
}

float FMossUtilities::ClampDeltaSeconds(const float DeltaSeconds, const float Maximum)
{
    if (!FMath::IsFinite(DeltaSeconds) || !FMath::IsFinite(Maximum))
    {
        return 0.0f;
    }
    return FMath::Clamp(DeltaSeconds, 0.0f, FMath::Max(0.0f, Maximum));
}

bool FMossUtilities::IsValidIdentifier(const FString& Identifier)
{
    if (Identifier.IsEmpty() || !(FChar::IsAlpha(Identifier[0]) || Identifier[0] == TEXT('_')))
    {
        return false;
    }

    for (const TCHAR Character : Identifier)
    {
        if (!(FChar::IsAlnum(Character) || Character == TEXT('_')))
        {
            return false;
        }
    }
    return true;
}
