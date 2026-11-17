#pragma once

#include "CoreMinimal.h"

/**
 * Unreal-friendly facade over Moss's actual Vec2/Vec3/Quat math types.
 * These functions call Moss vector/quaternion operations when the native SDK
 * and matching library are configured. They do not use a physics provider.
 */
class MOSSUNREAL_API FMossNativeMath final
{
public:
    static bool IsAvailable();
    static float Dot(const FVector& A, const FVector& B);
    static FVector Cross(const FVector& A, const FVector& B);
    static float Length(const FVector& Value);
    static FVector Normalize(const FVector& Value, const FVector& Fallback = FVector::ZeroVector);
    static FVector Min(const FVector& A, const FVector& B);
    static FVector Max(const FVector& A, const FVector& B);
    static FQuat NormalizeQuaternion(const FQuat& Value);
    static FQuat Slerp(const FQuat& A, const FQuat& B, float Alpha);
};
