#include "Math/MossNativeMath.h"
#include "Core/MossNative.h"

bool FMossNativeMath::IsAvailable()
{
    return MOSSUNREAL_NATIVE_API_AVAILABLE != 0;
}

float FMossNativeMath::Dot(const FVector& A, const FVector& B)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Vec3(static_cast<float>(A.X), static_cast<float>(A.Y), static_cast<float>(A.Z)).Dot(
        Vec3(static_cast<float>(B.X), static_cast<float>(B.Y), static_cast<float>(B.Z)));
#else
    return 0.0f;
#endif
}

FVector FMossNativeMath::Cross(const FVector& A, const FVector& B)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Vec3 Result = Vec3(static_cast<float>(A.X), static_cast<float>(A.Y), static_cast<float>(A.Z)).Cross(
        Vec3(static_cast<float>(B.X), static_cast<float>(B.Y), static_cast<float>(B.Z)));
    return FVector(Result.GetX(), Result.GetY(), Result.GetZ());
#else
    return FVector::ZeroVector;
#endif
}

float FMossNativeMath::Length(const FVector& Value)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Vec3(static_cast<float>(Value.X), static_cast<float>(Value.Y), static_cast<float>(Value.Z)).Length();
#else
    return 0.0f;
#endif
}

FVector FMossNativeMath::Normalize(const FVector& Value, const FVector& Fallback)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Vec3 NativeValue(static_cast<float>(Value.X), static_cast<float>(Value.Y), static_cast<float>(Value.Z));
    const Vec3 NativeFallback(static_cast<float>(Fallback.X), static_cast<float>(Fallback.Y), static_cast<float>(Fallback.Z));
    const Vec3 Result = NativeValue.NormalizedOr(NativeFallback);
    return FVector(Result.GetX(), Result.GetY(), Result.GetZ());
#else
    return Fallback;
#endif
}

FVector FMossNativeMath::Min(const FVector& A, const FVector& B)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Vec3 Result = Vec3::Min(Vec3(static_cast<float>(A.X), static_cast<float>(A.Y), static_cast<float>(A.Z)),
                                  Vec3(static_cast<float>(B.X), static_cast<float>(B.Y), static_cast<float>(B.Z)));
    return FVector(Result.GetX(), Result.GetY(), Result.GetZ());
#else
    return FVector(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y), FMath::Min(A.Z, B.Z));
#endif
}

FVector FMossNativeMath::Max(const FVector& A, const FVector& B)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Vec3 Result = Vec3::Max(Vec3(static_cast<float>(A.X), static_cast<float>(A.Y), static_cast<float>(A.Z)),
                                  Vec3(static_cast<float>(B.X), static_cast<float>(B.Y), static_cast<float>(B.Z)));
    return FVector(Result.GetX(), Result.GetY(), Result.GetZ());
#else
    return FVector(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y), FMath::Max(A.Z, B.Z));
#endif
}

FQuat FMossNativeMath::NormalizeQuaternion(const FQuat& Value)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Quat Native(static_cast<float>(Value.X), static_cast<float>(Value.Y), static_cast<float>(Value.Z), static_cast<float>(Value.W));
    const Quat Result = Native.Normalized();
    return FQuat(Result.GetX(), Result.GetY(), Result.GetZ(), Result.GetW());
#else
    return Value.GetNormalized();
#endif
}

FQuat FMossNativeMath::Slerp(const FQuat& A, const FQuat& B, const float Alpha)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    const Quat NativeA(static_cast<float>(A.X), static_cast<float>(A.Y), static_cast<float>(A.Z), static_cast<float>(A.W));
    const Quat NativeB(static_cast<float>(B.X), static_cast<float>(B.Y), static_cast<float>(B.Z), static_cast<float>(B.W));
    const Quat Result = NativeA.SLERP(NativeB, FMath::Clamp(Alpha, 0.0f, 1.0f)).Normalized();
    return FQuat(Result.GetX(), Result.GetY(), Result.GetZ(), Result.GetW());
#else
    return FQuat::Slerp(A, B, FMath::Clamp(Alpha, 0.0f, 1.0f));
#endif
}
