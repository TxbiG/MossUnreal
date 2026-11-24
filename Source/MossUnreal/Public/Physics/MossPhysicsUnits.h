#pragma once

#include "CoreMinimal.h"

/** Coordinate and unit conversion at the Unreal (cm, Z-up) / Moss (m, Y-up) boundary. */
struct MOSSUNREAL_API FMossPhysicsUnits final
{
    static constexpr double MossMetersPerUnrealCentimeter = 0.01;
    static constexpr double UnrealCentimetersPerMossMeter = 100.0;

    static double ToMossMeters(double UnrealCentimeters) { return UnrealCentimeters * MossMetersPerUnrealCentimeter; }
    static double ToUnrealCentimeters(double MossMeters) { return MossMeters * UnrealCentimetersPerMossMeter; }

    /** UE (X forward, Y right, Z up) -> Moss (X forward, Y up, Z left). */
    static FVector ToMossVector(const FVector& UnrealVector)
    {
        return FVector(UnrealVector.X, UnrealVector.Z, -UnrealVector.Y);
    }

    /** Moss (X forward, Y up, Z left) -> UE (X forward, Y right, Z up). */
    static FVector ToUnrealVector(const FVector& MossVector)
    {
        return FVector(MossVector.X, -MossVector.Z, MossVector.Y);
    }

    static FVector ToMossMeters(const FVector& UnrealCentimeters)
    {
        return ToMossVector(UnrealCentimeters) * MossMetersPerUnrealCentimeter;
    }

    static FVector ToUnrealCentimeters(const FVector& MossMeters)
    {
        return ToUnrealVector(MossMeters) * UnrealCentimetersPerMossMeter;
    }

    /** Conjugates the UE orientation by the fixed -90 degree X-axis basis rotation. */
    static FQuat ToMossRotation(const FQuat& UnrealRotation)
    {
        return FQuat(UnrealRotation.X, UnrealRotation.Z, -UnrealRotation.Y, UnrealRotation.W).GetNormalized();
    }

    static FQuat ToUnrealRotation(const FQuat& MossRotation)
    {
        return FQuat(MossRotation.X, -MossRotation.Z, MossRotation.Y, MossRotation.W).GetNormalized();
    }
};
