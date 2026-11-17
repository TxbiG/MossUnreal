#pragma once

#include "CoreMinimal.h"

/** Small, engine-friendly mathematical helpers; no Moss physics backend is required. */
struct MOSSUNREAL_API FMossMath final
{
    static bool NearlyEqual(double A, double B, double Tolerance = KINDA_SMALL_NUMBER);
    static double Remap(double Value, double InMin, double InMax, double OutMin, double OutMax, bool bClamp = false);
    static FVector SafeNormalize(const FVector& Value, const FVector& Fallback = FVector::ZeroVector);
    static FVector ClampMagnitude(const FVector& Value, double MaxMagnitude);
    static double SmoothStep(double A, double B, double Value);
};
