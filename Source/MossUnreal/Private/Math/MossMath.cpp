#include "Math/MossMath.h"

bool FMossMath::NearlyEqual(const double A, const double B, const double Tolerance)
{
    return FMath::Abs(A - B) <= FMath::Max(0.0, Tolerance);
}

double FMossMath::Remap(const double Value, const double InMin, const double InMax,
                        const double OutMin, const double OutMax, const bool bClamp)
{
    const double Range = InMax - InMin;
    if (FMath::IsNearlyZero(Range))
    {
        return OutMin;
    }

    double Alpha = (Value - InMin) / Range;
    if (bClamp)
    {
        Alpha = FMath::Clamp(Alpha, 0.0, 1.0);
    }
    return FMath::Lerp(OutMin, OutMax, Alpha);
}

FVector FMossMath::SafeNormalize(const FVector& Value, const FVector& Fallback)
{
    return Value.IsNearlyZero() ? Fallback : Value.GetSafeNormal();
}

FVector FMossMath::ClampMagnitude(const FVector& Value, const double MaxMagnitude)
{
    return Value.GetClampedToMaxSize(FMath::Max(0.0, MaxMagnitude));
}

double FMossMath::SmoothStep(const double A, const double B, const double Value)
{
    if (FMath::IsNearlyEqual(A, B))
    {
        return Value >= B ? 1.0 : 0.0;
    }
    const double T = FMath::Clamp((Value - A) / (B - A), 0.0, 1.0);
    return T * T * (3.0 - 2.0 * T);
}
