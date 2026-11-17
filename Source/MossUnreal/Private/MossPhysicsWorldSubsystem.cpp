#include "MossPhysicsWorldSubsystem.h"

#include "Core/MossRuntime.h"
#include "Core/MossRuntimeSubsystem.h"
#include "Diagnostics/MossDiagnostics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MossPhysicsBodyComponent.h"

void UMossPhysicsWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    EnsureNativeWorld();
}

void UMossPhysicsWorldSubsystem::Deinitialize()
{
    Systems.Shutdown();
    ClearBodyHandles();

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (Backend.IsValid() && NativeWorld.IsValid())
    {
        Backend->DestroyWorld(NativeWorld);
    }

    NativeWorld.Reset();
    bMossWorldReady = false;
    Accumulator = 0.0;
    Super::Deinitialize();
}

bool UMossPhysicsWorldSubsystem::EnsureNativeWorld()
{
    if (bMossWorldReady && NativeWorld.IsValid())
    {
        return true;
    }

    // A provider can be loaded by another plugin after UEngineSubsystem::Initialize.
    // Ask the lifecycle owner to retry instead of bypassing its shutdown ownership.
    if (!FMossRuntime::Get().IsInitialized())
    {
        if (GEngine)
        {
            if (UMossRuntimeSubsystem* RuntimeSubsystem = GEngine->GetEngineSubsystem<UMossRuntimeSubsystem>())
            {
                RuntimeSubsystem->TryInitializeRuntime();
            }
        }
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (!Backend.IsValid() || !Backend->IsInitialized())
    {
        return false;
    }

    FMossNativeWorldDescription Description;
    Description.FixedDeltaSeconds = FixedDeltaSeconds;
    if (UWorld* World = GetWorld())
    {
        Description.Gravity = FVector(0.0, 0.0, World->GetGravityZ());
    }

    FString Error;
    const bool bCreated = Backend->CreateWorld(Description, NativeWorld, Error);
    bMossWorldReady = bCreated && NativeWorld.IsValid();
    if (!bMossWorldReady)
    {
        if (NativeWorld.IsValid())
        {
            Backend->DestroyWorld(NativeWorld);
            NativeWorld.Reset();
        }
        if (!Error.IsEmpty())
        {
            FMossDiagnostics::Error(Error);
        }
        return false;
    }

    Accumulator = 0.0;
    bPendingBodiesNeedImmediateRetry = true;
    return true;
}

void UMossPhysicsWorldSubsystem::Tick(const float DeltaTime)
{
    if (!EnsureNativeWorld())
    {
        return;
    }

    if (bPendingBodiesNeedImmediateRetry)
    {
        TryCreatePendingBodies();
        bPendingBodiesNeedImmediateRetry = false;
        PendingBodyRetryAccumulator = 0.0;
    }
    else
    {
        PendingBodyRetryAccumulator += FMath::IsFinite(DeltaTime) ? FMath::Max(0.0f, DeltaTime) : 0.0;
        if (PendingBodyRetryAccumulator >= 1.0)
        {
            TryCreatePendingBodies();
            PendingBodyRetryAccumulator = 0.0;
        }
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (!Backend.IsValid())
    {
        InvalidateRegisteredBodyHandles();
        Systems.Shutdown();
        bMossWorldReady = false;
        NativeWorld.Reset();
        return;
    }

    const float SafeDeltaTime = FMath::IsFinite(DeltaTime) ? FMath::Max(0.0f, DeltaTime) : 0.0f;
    Accumulator += SafeDeltaTime;
    int32 Steps = 0;
    while (Accumulator >= FixedDeltaSeconds && Steps < MaxSubsteps)
    {
        SynchronizeKinematicBodies();

        FString Error;
        if (!Backend->StepWorld(NativeWorld, FixedDeltaSeconds, Error))
        {
            FMossDiagnostics::Error(Error.IsEmpty() ? TEXT("Moss physics world stepping failed.") : Error);
            Systems.Shutdown();
            InvalidateRegisteredBodyHandles();
            Backend->DestroyWorld(NativeWorld);
            NativeWorld.Reset();
            bMossWorldReady = false;
            Accumulator = 0.0;
            return;
        }

        if (!Systems.IsInitialized())
        {
            FString SystemError;
            if (!Systems.Initialize(SystemError) && !SystemError.IsEmpty())
            {
                FMossDiagnostics::Error(SystemError);
            }
        }
        Systems.Tick(FixedDeltaSeconds);

        Accumulator -= FixedDeltaSeconds;
        ++Steps;
    }

    if (Steps == MaxSubsteps && Accumulator >= FixedDeltaSeconds)
    {
        // Avoid a runaway catch-up spiral after a long frame. The dropped time is explicit.
        Accumulator = FMath::Fmod(Accumulator, static_cast<double>(FixedDeltaSeconds));
    }

    SynchronizeDynamicBodies();
}

TStatId UMossPhysicsWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UMossPhysicsWorldSubsystem, STATGROUP_Tickables);
}

bool UMossPhysicsWorldSubsystem::IsTickable() const
{
    // Keep the subsystem alive while the optional provider is loading so it can
    // initialize the runtime/world and materialize queued body components later.
    return !IsTemplate();
}

void UMossPhysicsWorldSubsystem::ConfigureStepping(const float InFixedDeltaSeconds, const int32 InMaxSubsteps)
{
    FixedDeltaSeconds = FMath::Clamp(InFixedDeltaSeconds, 1.0f / 1000.0f, 0.25f);
    MaxSubsteps = FMath::Clamp(InMaxSubsteps, 1, 32);
    Accumulator = 0.0;
}

bool UMossPhysicsWorldSubsystem::RegisterBody(UMossPhysicsBodyComponent* Component, FString& OutError)
{
    if (!IsValid(Component))
    {
        OutError = TEXT("Cannot register a Moss body: the component is invalid.");
        return false;
    }

    RegisteredBodies.AddUnique(TWeakObjectPtr<UMossPhysicsBodyComponent>(Component));
    if (!EnsureNativeWorld())
    {
        // Registration is queued. Tick retries after a provider is registered and initialized.
        bPendingBodiesNeedImmediateRetry = true;
        OutError.Reset();
        return true;
    }

    const bool bCreated = CreateBodyForComponent(Component, OutError);
    if (!bCreated)
    {
        bPendingBodiesNeedImmediateRetry = true;
    }
    return bCreated;
}

bool UMossPhysicsWorldSubsystem::CreateBodyForComponent(UMossPhysicsBodyComponent* Component, FString& OutError)
{
    if (!IsValid(Component) || !bMossWorldReady || !NativeWorld.IsValid())
    {
        OutError = TEXT("Cannot create a Moss body until its component and native world are ready.");
        return false;
    }
    if (Component->NativeBody.IsValid())
    {
        OutError.Reset();
        return true;
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (!Backend.IsValid())
    {
        OutError = TEXT("The native Moss backend is no longer available.");
        return false;
    }

    FMossNativeBodyHandle Body;
    const bool bCreated = Backend->CreateBody(NativeWorld, Component->BuildNativeDescription(), Body, OutError);
    if (!bCreated || !Body.IsValid())
    {
        if (Body.IsValid())
        {
            Backend->DestroyBody(NativeWorld, Body);
        }
        if (OutError.IsEmpty())
        {
            OutError = TEXT("The Moss backend failed to create a valid body handle.");
        }
        return false;
    }

    Component->SetNativeBodyHandle(Body);
    OutError.Reset();
    return true;
}

void UMossPhysicsWorldSubsystem::TryCreatePendingBodies()
{
    RegisteredBodies.RemoveAll([](const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry)
    {
        return !Entry.IsValid();
    });

    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            if (!Component->NativeBody.IsValid())
            {
                FString IgnoredError;
                CreateBodyForComponent(Component, IgnoredError);
            }
        }
    }
}

void UMossPhysicsWorldSubsystem::InvalidateRegisteredBodyHandles()
{
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            Component->SetNativeBodyHandle(FMossNativeBodyHandle());
        }
    }
    bPendingBodiesNeedImmediateRetry = true;
}

void UMossPhysicsWorldSubsystem::UnregisterBody(UMossPhysicsBodyComponent* Component)
{
    if (Component == nullptr)
    {
        return;
    }

    const FMossNativeBodyHandle Body = Component->NativeBody;
    Component->SetNativeBodyHandle(FMossNativeBodyHandle());

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (Backend.IsValid() && NativeWorld.IsValid() && Body.IsValid())
    {
        Backend->DestroyBody(NativeWorld, Body);
    }

    RegisteredBodies.RemoveAll([Component](const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry)
    {
        return !Entry.IsValid() || Entry.Get() == Component;
    });
}

bool UMossPhysicsWorldSubsystem::Raycast(const FVector& Start, const FVector& End,
                                         FMossPhysicsRaycastHit& OutHit, FString& OutError) const
{
    OutHit = FMossPhysicsRaycastHit();
    if (!bMossWorldReady || !NativeWorld.IsValid())
    {
        OutError = TEXT("Moss raycast requested while the Moss world is unavailable.");
        return false;
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (!Backend.IsValid())
    {
        OutError = TEXT("The native Moss backend is unavailable.");
        return false;
    }

    FMossNativeRaycastQuery Query;
    Query.Start = Start;
    Query.End = End;
    FMossNativeRaycastHit NativeHit;
    if (!Backend->Raycast(NativeWorld, Query, NativeHit, OutError))
    {
        return false;
    }

    OutHit.bBlockingHit = NativeHit.bBlockingHit;
    OutHit.Location = NativeHit.Location;
    OutHit.Normal = NativeHit.Normal;
    OutHit.Distance = static_cast<float>(NativeHit.Distance);
    OutHit.NativeBody = NativeHit.Body;
    OutError.Reset();
    return true;
}

void UMossPhysicsWorldSubsystem::SynchronizeKinematicBodies()
{
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            if (Component->GetBodyMode() == EMossBodyMode::Kinematic)
            {
                Component->SynchronizeToNative(this);
            }
        }
    }
}

void UMossPhysicsWorldSubsystem::SynchronizeDynamicBodies()
{
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            if (Component->GetBodyMode() == EMossBodyMode::Dynamic)
            {
                Component->SynchronizeFromNative(this);
            }
        }
    }
}

void UMossPhysicsWorldSubsystem::ClearBodyHandles()
{
    const TArray<TWeakObjectPtr<UMossPhysicsBodyComponent>> BodiesCopy = RegisteredBodies;
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : BodiesCopy)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            UnregisterBody(Component);
        }
    }
    RegisteredBodies.Reset();
    PendingBodyRetryAccumulator = 0.0;
    bPendingBodiesNeedImmediateRetry = false;
}
