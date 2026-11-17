#pragma once

#include "CoreMinimal.h"

/**
 * Explicit length conversion helpers for the Unreal <-> Moss physics boundary.
 * Unreal world positions/dimensions use centimetres; Moss's physics API documents
 * its collision tolerances and convex radii in metres. Orientation/handedness
 * conversion remains a separate provider responsibility.
 */
struct MOSSUNREAL_API FMossPhysicsUnits final
{
    static constexpr double MossMetersPerUnrealCentimeter = 0.01;
    static constexpr double UnrealCentimetersPerMossMeter = 100.0;

    static double ToMossMeters(double UnrealCentimeters)
    {
        return UnrealCentimeters * MossMetersPerUnrealCentimeter;
    }

    static double ToUnrealCentimeters(double MossMeters)
    {
        return MossMeters * UnrealCentimetersPerMossMeter;
    }

    static FVector ToMossMeters(const FVector& UnrealCentimeters)
    {
        return UnrealCentimeters * MossMetersPerUnrealCentimeter;
    }

    static FVector ToUnrealCentimeters(const FVector& MossMeters)
    {
        return MossMeters * UnrealCentimetersPerMossMeter;
    }
};
