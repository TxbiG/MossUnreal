#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Physics/MossPhysicsTypes.h"
#include "MossPhysicsBodyComponent.generated.h"

class FMossPhysicsBodyDesc;
class UMossPhysicsWorldSubsystem;
class UPrimitiveComponent;

struct FMossChaosPrimitiveState
{
    TWeakObjectPtr<UPrimitiveComponent> Primitive;
    ECollisionEnabled::Type PreviousCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    bool bWasSimulatingPhysics = false;
};

UENUM(BlueprintType)
enum class EMossChaosConflictPolicy : uint8
{
    DisableChaosPhysicsCollision UMETA(DisplayName="Disable Chaos Physics Collision on Owner"),
    RejectMossBody UMETA(DisplayName="Reject Moss Body When Chaos Physics Is Enabled")
};

UENUM(BlueprintType)
enum class EMossPhysicsMotionQuality : uint8
{
    Discrete UMETA(DisplayName="Discrete"),
    LinearCast UMETA(DisplayName="Linear Cast")
};

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EMossPhysicsDOF : uint8
{
    TranslationX = 1 << 0,
    TranslationY = 1 << 1,
    TranslationZ = 1 << 2,
    RotationX = 1 << 3,
    RotationY = 1 << 4,
    RotationZ = 1 << 5
};
ENUM_CLASS_FLAGS(EMossPhysicsDOF)

/**
 * Unreal-facing adapter for one body in the per-world Moss PhysicsSystem.
 * Collision shapes are gathered from simple UBox/USphere/UCapsule components
 * on the owning Actor and submitted as one Moss compound body.
 */
UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossPhysicsBodyComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UMossPhysicsBodyComponent();

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    bool IsMossBodyReady() const { return NativeBody.IsValid(); }

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    int64 GetNativeBodyId() const { return NativeBody.IsValid() ? static_cast<int64>(NativeBody.Value) : -1; }

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    EMossBodyMode GetBodyMode() const { return BodyMode; }

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool RecreateMossBody();

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool WakeBody();

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool AddForce(const FVector& ForceNewtons);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool AddImpulse(const FVector& ImpulseNewtonSeconds);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool AddTorque(const FVector& TorqueNewtonMeters);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetLinearVelocity(const FVector& VelocityCentimetersPerSecond);

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    FVector GetLinearVelocity() const;

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetAngularVelocity(const FVector& AngularVelocityRadiansPerSecond);

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    FVector GetAngularVelocity() const;

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetFriction(float NewFriction);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetRestitution(float NewRestitution);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetGravityFactor(float NewGravityFactor);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetMass(float NewMassKg);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetLinearDamping(float NewDamping);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetAngularDamping(float NewDamping);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetMaxLinearVelocity(float MetersPerSecond);

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    float GetMaxLinearVelocity() const { return MaxLinearVelocityMetersPerSecond; }

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetMaxAngularVelocity(float RadiansPerSecond);

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    float GetMaxAngularVelocity() const { return MaxAngularVelocityRadiansPerSecond; }

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetAllowSleeping(bool bNewAllowSleeping);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetMotionQuality(EMossPhysicsMotionQuality NewMotionQuality);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetEnhancedInternalEdgeRemoval(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetApplyGyroscopicForce(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetSolverSteps(int32 VelocitySteps, int32 PositionSteps);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool SetSensor(bool bNewSensor);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Collision")
    bool SetCollisionLayer(FName NewCollisionLayer);

    UPROPERTY(BlueprintAssignable, Category="Moss|Physics|Events")
    FMossPhysicsContactEvent OnMossContactAdded;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    friend class UMossPhysicsWorldSubsystem;

    bool BuildNativeDescription(FMossPhysicsBodyDesc& OutDescription, FString& OutError) const;
    bool ResolveChaosOwnership(FString& OutError);
    void RestoreChaosOwnership();
    void SetNativeBodyId(uint32 Value) { NativeBody.Value = Value; }
    void SynchronizeToNative(UMossPhysicsWorldSubsystem* WorldSubsystem, float DeltaSeconds);
    void SynchronizeFromNative(UMossPhysicsWorldSubsystem* WorldSubsystem);
    void DrawDebugBody(UWorld* World, float Duration) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    EMossBodyMode BodyMode = EMossBodyMode::Dynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true"))
    FName CollisionLayer = TEXT("WorldDynamic");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true"))
    bool bGatherOwnerCollisionComponents = true;

    /** Prevents one Actor's primitives from participating in both Moss and Chaos physical collision. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Integration", meta=(AllowPrivateAccess="true"))
    EMossChaosConflictPolicy ChaosConflictPolicy = EMossChaosConflictPolicy::DisableChaosPhysicsCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true"))
    EMossCollisionShape FallbackShape = EMossCollisionShape::Box;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    FVector BoxHalfExtents = FVector(50.0, 50.0, 50.0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    float CapsuleHalfHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Collision", meta=(AllowPrivateAccess="true"))
    TArray<FVector> ConvexHullVertices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0.001"))
    float MassKg = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Material", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float Friction = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Material", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
    float Restitution = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float LinearDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float AngularDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    float GravityFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float MaxLinearVelocityMetersPerSecond = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float MaxAngularVelocityRadiansPerSecond = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", Bitmask, BitmaskEnum="EMossPhysicsDOF"))
    int32 AllowedDOFs = 0x3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0", ClampMax="255"))
    int32 VelocitySolverSteps = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true", ClampMin="0", ClampMax="255"))
    int32 PositionSolverSteps = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    EMossPhysicsMotionQuality MotionQuality = EMossPhysicsMotionQuality::Discrete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    bool bSensor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    bool bAllowSleeping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    bool bApplyGyroscopicForce = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    bool bEnhancedInternalEdgeRemoval = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Body", meta=(AllowPrivateAccess="true"))
    bool bSynchronizeTransform = true;

    FMossPhysicsBodyId NativeBody;
    TArray<FMossChaosPrimitiveState> PreviousChaosPrimitiveStates;
};
