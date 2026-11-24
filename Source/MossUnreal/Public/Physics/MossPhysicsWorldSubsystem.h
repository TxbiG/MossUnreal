#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Systems/MossSystem.h"
#include "Physics/MossPhysicsTypes.h"
#include "MossPhysicsWorldSubsystem.generated.h"

class FMossPhysicsScene;
class UMossPhysicsBodyComponent;
class AActor;

/**
 * Owns one independent Moss PhysicsSystem per UWorld. Chaos remains available
 * to other actors; Moss bodies are simulated only by this subsystem.
 */
UCLASS()
class MOSSUNREAL_API UMossPhysicsWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual ~UMossPhysicsWorldSubsystem() override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableWhenPaused() const override { return false; }

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    bool IsMossWorldReady() const { return bMossWorldReady; }

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    float GetFixedDeltaSeconds() const { return FixedDeltaSeconds; }

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    void ConfigureStepping(float InFixedDeltaSeconds, int32 InMaxSubsteps);

    /** Scans the current world for actors tagged with the configured Moss static/dynamic tags. */
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Integration")
    int32 RefreshTaggedActors();

    UFUNCTION(BlueprintCallable, Category="Moss|Physics", meta=(DisplayName="Moss Raycast"))
    bool Raycast(const FVector& Start, const FVector& End, FMossPhysicsRaycastHit& OutHit);

    UPROPERTY(BlueprintAssignable, Category="Moss|Physics|Events")
    FMossPhysicsContactEvent OnMossContactAdded;

    /** Register/unregister are called by Moss physics components. */
    bool RegisterBody(UMossPhysicsBodyComponent* Component, FString& OutError);
    void UnregisterBody(UMossPhysicsBodyComponent* Component);

    /** World-scoped ordinary C++ systems; no Actor or Component inheritance required. */
    FMossSystemRegistry& GetSystems() { return Systems; }
    const FMossSystemRegistry& GetSystems() const { return Systems; }

private:
    friend class UMossPhysicsBodyComponent;

    bool EnsureNativeScene(FString& OutError);
    void HandleActorSpawned(AActor* Actor);
    bool RegisterActorFromTags(AActor* Actor);
    void TryCreatePendingBodies();
    bool CreateBodyForComponent(UMossPhysicsBodyComponent* Component, FString& OutError);
    void SynchronizeKinematicBodies();
    void SynchronizeDynamicBodies();
    void DispatchContactEvents();
    void ClearBodyHandles();
    void InvalidateBodyHandlesPreserveRegistrations();
    bool WakeBody(FMossPhysicsBodyId Body, FString& OutError);
    bool ApplyForce(FMossPhysicsBodyId Body, const FVector& ForceNewtons, FString& OutError);
    bool ApplyImpulse(FMossPhysicsBodyId Body, const FVector& ImpulseNewtonSeconds, FString& OutError);
    bool ApplyTorque(FMossPhysicsBodyId Body, const FVector& TorqueNewtonMeters, FString& OutError);
    bool SetLinearVelocity(FMossPhysicsBodyId Body, const FVector& VelocityCmPerSecond, FString& OutError);
    bool GetLinearVelocity(FMossPhysicsBodyId Body, FVector& OutVelocityCmPerSecond, FString& OutError) const;
    bool SetAngularVelocity(FMossPhysicsBodyId Body, const FVector& AngularVelocityRadiansPerSecond, FString& OutError);
    bool GetAngularVelocity(FMossPhysicsBodyId Body, FVector& OutAngularVelocityRadiansPerSecond, FString& OutError) const;
    bool SetFriction(FMossPhysicsBodyId Body, float Friction, FString& OutError);
    bool SetRestitution(FMossPhysicsBodyId Body, float Restitution, FString& OutError);
    bool SetGravityFactor(FMossPhysicsBodyId Body, float GravityFactor, FString& OutError);
    bool SetMass(FMossPhysicsBodyId Body, float MassKg, FString& OutError);
    bool SetLinearDamping(FMossPhysicsBodyId Body, float Damping, FString& OutError);
    bool SetAngularDamping(FMossPhysicsBodyId Body, float Damping, FString& OutError);
    bool SetMaxLinearVelocity(FMossPhysicsBodyId Body, float MetersPerSecond, FString& OutError);
    bool SetMaxAngularVelocity(FMossPhysicsBodyId Body, float RadiansPerSecond, FString& OutError);
    bool SetAllowSleeping(FMossPhysicsBodyId Body, bool bAllowSleeping, FString& OutError);
    bool SetMotionQuality(FMossPhysicsBodyId Body, bool bLinearCast, FString& OutError);
    bool SetEnhancedInternalEdgeRemoval(FMossPhysicsBodyId Body, bool bEnabled, FString& OutError);
    bool SetApplyGyroscopicForce(FMossPhysicsBodyId Body, bool bEnabled, FString& OutError);
    bool SetSolverSteps(FMossPhysicsBodyId Body, uint32 VelocitySteps, uint32 PositionSteps, FString& OutError);
    bool SetSensor(FMossPhysicsBodyId Body, bool bSensor, FString& OutError);
    bool SetObjectLayer(FMossPhysicsBodyId Body, int32 ObjectLayer, FString& OutError);

    TUniquePtr<FMossPhysicsScene> PhysicsScene;
    TArray<TWeakObjectPtr<UMossPhysicsBodyComponent>> RegisteredBodies;
    /** O(1) contact-to-component lookup; entries are removed when BodyIDs are invalidated. */
    TMap<uint32, TWeakObjectPtr<UMossPhysicsBodyComponent>> BodyComponentsById;
    FMossSystemRegistry Systems;
    float FixedDeltaSeconds = 1.0f / 60.0f;
    int32 MaxSubsteps = 4;
    int32 CollisionSteps = 1;
    double Accumulator = 0.0;
    double PendingBodyRetryAccumulator = 0.0;
    bool bPendingBodiesNeedImmediateRetry = false;
    bool bMossWorldReady = false;
    bool bInitializationWarningIssued = false;
    bool bSystemsInitializationAttempted = false;
    bool bHasScannedActorTags = false;
    FDelegateHandle ActorSpawnedHandle;
};
