#pragma once

#include "CoreMinimal.h"
#include "MossPhysicsTypes.generated.h"

/** Native Moss BodyID encoded as the low 32-bit Jolt-derived index/sequence value. */
struct MOSSUNREAL_API FMossPhysicsBodyId
{
    static constexpr uint32 InvalidValue = 0xffffffffu;
    uint32 Value = InvalidValue;

    bool IsValid() const { return Value != InvalidValue; }
    void Reset() { Value = InvalidValue; }
    friend bool operator==(const FMossPhysicsBodyId& A, const FMossPhysicsBodyId& B) { return A.Value == B.Value; }
    friend bool operator!=(const FMossPhysicsBodyId& A, const FMossPhysicsBodyId& B) { return !(A == B); }
};

UENUM(BlueprintType)
enum class EMossBodyMode : uint8
{
    Static UMETA(DisplayName="Static"),
    Kinematic UMETA(DisplayName="Kinematic"),
    Dynamic UMETA(DisplayName="Dynamic")
};

UENUM(BlueprintType)
enum class EMossCollisionShape : uint8
{
    Box UMETA(DisplayName="Box"),
    Sphere UMETA(DisplayName="Sphere"),
    Capsule UMETA(DisplayName="Capsule"),
    ConvexHull UMETA(DisplayName="Convex Hull")
};

USTRUCT(BlueprintType)
struct MOSSUNREAL_API FMossPhysicsRaycastHit
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    bool bBlockingHit = false;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    FVector Normal = FVector::UpVector;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    float Distance = 0.0f;

    /** Raw Moss BodyID (index + sequence); invalid is 0xffffffff. */
    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    int64 NativeBodyId = -1;
};

USTRUCT(BlueprintType)
struct MOSSUNREAL_API FMossPhysicsContact
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    int64 BodyA = -1;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    int64 BodyB = -1;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    FVector Point = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    FVector Normal = FVector::UpVector;

    UPROPERTY(BlueprintReadOnly, Category="Moss|Physics")
    float PenetrationDepth = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMossPhysicsContactEvent, const FMossPhysicsContact&, Contact);
