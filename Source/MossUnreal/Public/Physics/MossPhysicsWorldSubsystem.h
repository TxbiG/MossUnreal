#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/MossBackend.h"
#include "Systems/MossSystem.h"
#include "Physics/MossPhysicsTypes.h"
#include "MossPhysicsWorldSubsystem.generated.h"

class UMossPhysicsBodyComponent;

/**
 * Per-UWorld owner for one Moss simulation world. Uses a fixed-step accumulator
 * and keeps native world/body handles on the Moss side of the ownership boundary.
 */
UCLASS()
class MOSSUNREAL_API UMossPhysicsWorldSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
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

    /** Configure deterministic stepping before or during play. */
    void ConfigureStepping(float InFixedDeltaSeconds, int32 InMaxSubsteps);

    /** Register/unregister are called by Moss physics components. */
    bool RegisterBody(UMossPhysicsBodyComponent* Component, FString& OutError);
    void UnregisterBody(UMossPhysicsBodyComponent* Component);

    bool Raycast(const FVector& Start, const FVector& End, FMossPhysicsRaycastHit& OutHit, FString& OutError) const;

    /** World-scoped ordinary C++ systems; no Actor or Component inheritance required. */
    FMossSystemRegistry& GetSystems() { return Systems; }
    const FMossSystemRegistry& GetSystems() const { return Systems; }

private:
    friend class UMossPhysicsBodyComponent;

    void SynchronizeKinematicBodies();
    void SynchronizeDynamicBodies();
    bool EnsureNativeWorld();
    void TryCreatePendingBodies();
    void InvalidateRegisteredBodyHandles();
    bool CreateBodyForComponent(UMossPhysicsBodyComponent* Component, FString& OutError);
    void ClearBodyHandles();

    FMossNativeWorldHandle NativeWorld;
    TArray<TWeakObjectPtr<UMossPhysicsBodyComponent>> RegisteredBodies;
    FMossSystemRegistry Systems;
    float FixedDeltaSeconds = 1.0f / 60.0f;
    int32 MaxSubsteps = 4;
    double Accumulator = 0.0;
    double PendingBodyRetryAccumulator = 0.0;
    bool bPendingBodiesNeedImmediateRetry = false;
    bool bMossWorldReady = false;
};
