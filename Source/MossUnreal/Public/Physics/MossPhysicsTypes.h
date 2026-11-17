#pragma once

#include "CoreMinimal.h"
#include "Core/MossBackend.h"

#include "MossPhysicsTypes.generated.h"

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
    Capsule UMETA(DisplayName="Capsule")
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

    /** Opaque native body handle for C++ consumers; deliberately not Blueprint-reflected. */
    FMossNativeBodyHandle NativeBody;
};
