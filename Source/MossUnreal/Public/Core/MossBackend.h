#pragma once

#include "CoreMinimal.h"

/** Opaque handle owned by the registered native Moss backend. Zero is always invalid. */
struct MOSSUNREAL_API FMossNativeHandle
{
    uint64 Value = 0;

    bool IsValid() const { return Value != 0; }
    void Reset() { Value = 0; }
    friend bool operator==(const FMossNativeHandle& A, const FMossNativeHandle& B) { return A.Value == B.Value; }
    friend bool operator!=(const FMossNativeHandle& A, const FMossNativeHandle& B) { return !(A == B); }
};

using FMossNativeWorldHandle = FMossNativeHandle;
using FMossNativeBodyHandle = FMossNativeHandle;

/** Metadata reported by a backend after its provider ABI/version handshake succeeds. */
struct MOSSUNREAL_API FMossBackendInfo
{
    FString Name;
    FString Version;
    int32 AbiVersion = 0;
    TSet<FName> Capabilities;
};

/** Runtime options passed to a native backend. */
struct MOSSUNREAL_API FMossRuntimeConfig
{
    bool bEnableDiagnostics = true;
    bool bEnableValidation = true;
    int32 ExpectedBackendAbi = 1;
};

/** World settings expressed in Unreal world units (centimetres and cm/s^2). A backend owns conversion. */
struct MOSSUNREAL_API FMossNativeWorldDescription
{
    FVector Gravity = FVector(0.0, 0.0, -980.0);
    double FixedDeltaSeconds = 1.0 / 60.0;
};

enum class EMossNativeBodyMode : uint8
{
    Static,
    Kinematic,
    Dynamic
};

enum class EMossNativeShape : uint8
{
    Box,
    Sphere,
    Capsule
};

/** Basic primitive body description. Shape dimensions use Unreal world units (centimetres). InitialTransform scale is normalized to one. */
struct MOSSUNREAL_API FMossNativeBodyDescription
{
    EMossNativeBodyMode Mode = EMossNativeBodyMode::Dynamic;
    EMossNativeShape Shape = EMossNativeShape::Box;
    FTransform InitialTransform = FTransform::Identity;
    FVector BoxHalfExtents = FVector(50.0, 50.0, 50.0);
    double Radius = 50.0;
    double HalfHeight = 100.0;
    double Mass = 1.0;
    double LinearDamping = 0.01;
    double AngularDamping = 0.01;
    bool bUseGravity = true;
};

/** Ray endpoints are Unreal world positions in centimetres. */
struct MOSSUNREAL_API FMossNativeRaycastQuery
{
    FVector Start = FVector::ZeroVector;
    FVector End = FVector(0.0, 0.0, -1000.0);
    bool bTraceComplex = false;
};

/** Hit location and distance are returned in Unreal world units (centimetres). */
struct MOSSUNREAL_API FMossNativeRaycastHit
{
    bool bBlockingHit = false;
    FVector Location = FVector::ZeroVector;
    FVector Normal = FVector::UpVector;
    double Distance = 0.0;
    FMossNativeBodyHandle Body;
};

/**
 * Implement this interface in the native Moss bridge that is built against the
 * real public Moss headers. Do not include Moss private headers in this plugin.
 *
 * Contract:
 * - Handles are opaque and backend-owned; Unreal must never free them.
 * - All transforms, gravity, ray endpoints and shape dimensions arrive in Unreal units.
 * - Convert Unreal centimetres to Moss physics metres (0.01 scale) in one place;
 *   FMossPhysicsUnits provides the length conversion helpers.
 * - The provider separately verifies coordinate handedness, up-axis and quaternion conversion.
 * - Calls are made on the game thread unless a method explicitly documents otherwise.
 * - No exception may cross this interface boundary.
 */
class MOSSUNREAL_API IMossBackend
{
public:
    virtual ~IMossBackend() = default;

    virtual bool Initialize(const FMossRuntimeConfig& Config, FString& OutError) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;
    virtual FMossBackendInfo GetInfo() const = 0;

    virtual bool CreateWorld(const FMossNativeWorldDescription& Description,
                             FMossNativeWorldHandle& OutWorld,
                             FString& OutError) = 0;
    virtual void DestroyWorld(FMossNativeWorldHandle World) = 0;
    virtual bool StepWorld(FMossNativeWorldHandle World, double DeltaSeconds, FString& OutError) = 0;

    virtual bool CreateBody(FMossNativeWorldHandle World,
                            const FMossNativeBodyDescription& Description,
                            FMossNativeBodyHandle& OutBody,
                            FString& OutError) = 0;
    virtual void DestroyBody(FMossNativeWorldHandle World, FMossNativeBodyHandle Body) = 0;
    virtual bool SetBodyTransform(FMossNativeWorldHandle World,
                                  FMossNativeBodyHandle Body,
                                  const FTransform& Transform,
                                  FString& OutError) = 0;
    virtual bool GetBodyTransform(FMossNativeWorldHandle World,
                                  FMossNativeBodyHandle Body,
                                  FTransform& OutTransform,
                                  FString& OutError) const = 0;
    virtual bool Raycast(FMossNativeWorldHandle World,
                         const FMossNativeRaycastQuery& Query,
                         FMossNativeRaycastHit& OutHit,
                         FString& OutError) const = 0;
};

/** Factory registered by the optional native Moss bridge module. */
using FMossBackendFactory = TFunction<TSharedPtr<IMossBackend, ESPMode::ThreadSafe>()>;
