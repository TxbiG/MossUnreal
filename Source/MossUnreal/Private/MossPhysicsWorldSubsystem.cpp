#include "Physics/MossPhysicsWorldSubsystem.h"

#include "Physics/MossPhysicsScene.h"
#include "Physics/MossPhysicsSettings.h"
#include "Physics/MossPhysicsBodyComponent.h"
#include "Diagnostics/MossDiagnostics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"

UMossPhysicsWorldSubsystem::~UMossPhysicsWorldSubsystem() = default;

void UMossPhysicsWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    if (Settings)
    {
        FixedDeltaSeconds = FMath::Clamp(Settings->FixedTimeStep, 1.0f / 1000.0f, 0.25f);
        MaxSubsteps = FMath::Clamp(Settings->MaxSubsteps, 1, 32);
        CollisionSteps = FMath::Clamp(Settings->CollisionSteps, 1, 16);
    }

    PhysicsScene = MakeUnique<FMossPhysicsScene>();

    if (Settings && Settings->bEnableActorTagDiscovery)
    {
        if (UWorld* World = GetWorld())
        {
            ActorSpawnedHandle = World->AddOnActorSpawnedHandler(
                FOnActorSpawned::FDelegate::CreateUObject(this, &UMossPhysicsWorldSubsystem::HandleActorSpawned));
        }
    }
    // The direct native integration is opt-in at build time. A default build
    // should not emit a startup warning merely because Moss Physics is disabled.
    if (FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        FString Error;
        if (!EnsureNativeScene(Error) && !Error.IsEmpty())
        {
            FMossDiagnostics::Warning(Error);
            bInitializationWarningIssued = true;
        }
    }
}

void UMossPhysicsWorldSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (ActorSpawnedHandle.IsValid())
        {
            World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
            ActorSpawnedHandle.Reset();
        }
    }

    Systems.Shutdown();
    ClearBodyHandles();

    if (PhysicsScene)
    {
        PhysicsScene->Shutdown();
        PhysicsScene.Reset();
    }

    bMossWorldReady = false;
    bSystemsInitializationAttempted = false;
    bHasScannedActorTags = false;
    Accumulator = 0.0;
    PendingBodyRetryAccumulator = 0.0;
    Super::Deinitialize();
}

bool UMossPhysicsWorldSubsystem::EnsureNativeScene(FString& OutError)
{
    if (bMossWorldReady && PhysicsScene && PhysicsScene->IsInitialized())
    {
        OutError.Reset();
        return true;
    }

    if (!PhysicsScene)
    {
        PhysicsScene = MakeUnique<FMossPhysicsScene>();
    }

    if (!FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        OutError = TEXT("Moss Physics is not compiled into MossUnreal. Set MOSS_ENABLE_PHYSICS=1 and MOSS_PHYSICS_HEADERS_READY=1 only after the Moss Physics header dependency chain has been repaired and verified. See Docs/MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md.");
        return false;
    }

    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    if (!Settings)
    {
        OutError = TEXT("Moss Physics project settings could not be loaded.");
        return false;
    }

    if (!Settings->ValidateLayers(OutError))
    {
        return false;
    }

    FMossPhysicsSceneConfig Config;
    Config.MaxBodies = Settings->MaxBodies;
    Config.MaxBodyPairs = Settings->MaxBodyPairs;
    Config.MaxContactConstraints = Settings->MaxContactConstraints;
    Config.AllocatorMegabytes = Settings->TemporaryAllocatorMegabytes;
    Config.BroadPhaseLayerCount = Settings->BroadPhaseLayerCount;
    Config.ObjectLayers = Settings->ObjectLayers;

    FVector Gravity(0.0, 0.0, -980.0);
    if (UWorld* World = GetWorld())
    {
        Gravity.Z = World->GetGravityZ();
    }

    if (!PhysicsScene->Initialize(Config, Gravity, OutError))
    {
        bMossWorldReady = false;
        return false;
    }

    bMossWorldReady = PhysicsScene->IsInitialized();
    Accumulator = 0.0;
    PendingBodyRetryAccumulator = 0.0;
    bPendingBodiesNeedImmediateRetry = true;
    bInitializationWarningIssued = false;
    bSystemsInitializationAttempted = false;
    OutError.Reset();
    return bMossWorldReady;
}

void UMossPhysicsWorldSubsystem::Tick(const float DeltaTime)
{
    if (!FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        return;
    }

    FString InitError;
    if (!EnsureNativeScene(InitError))
    {
        if (!InitError.IsEmpty() && !bInitializationWarningIssued)
        {
            FMossDiagnostics::Warning(InitError);
            bInitializationWarningIssued = true;
        }
        return;
    }

    if (!bHasScannedActorTags)
    {
        RefreshTaggedActors();
        bHasScannedActorTags = true;
    }

    if (bPendingBodiesNeedImmediateRetry)
    {
        TryCreatePendingBodies();
        bPendingBodiesNeedImmediateRetry = false;
        PendingBodyRetryAccumulator = 0.0;
    }
    else
    {
        PendingBodyRetryAccumulator += FMath::IsFinite(DeltaTime) ? FMath::Max(0.0f, DeltaTime) : 0.0f;
        if (PendingBodyRetryAccumulator >= 1.0)
        {
            TryCreatePendingBodies();
            PendingBodyRetryAccumulator = 0.0;
        }
    }

    const float SafeDeltaTime = FMath::IsFinite(DeltaTime) ? FMath::Max(0.0f, DeltaTime) : 0.0f;
    Accumulator += SafeDeltaTime;

    int32 Steps = 0;
    while (Accumulator >= FixedDeltaSeconds && Steps < MaxSubsteps)
    {
        SynchronizeKinematicBodies();

        FString StepError;
        if (!PhysicsScene->Step(FixedDeltaSeconds, CollisionSteps, StepError))
        {
            FMossDiagnostics::Error(StepError.IsEmpty() ? TEXT("Moss PhysicsSystem::Update failed.") : StepError);
            Systems.Shutdown();
            InvalidateBodyHandlesPreserveRegistrations();
            PhysicsScene->Shutdown();
            bMossWorldReady = false;
            bSystemsInitializationAttempted = false;
            bPendingBodiesNeedImmediateRetry = true;
            Accumulator = 0.0;
            return;
        }

        if (!bSystemsInitializationAttempted)
        {
            FString SystemError;
            if (!Systems.Initialize(SystemError) && !SystemError.IsEmpty())
            {
                FMossDiagnostics::Error(SystemError);
            }
            bSystemsInitializationAttempted = true;
        }
        Systems.Tick(FixedDeltaSeconds);
        DispatchContactEvents();

        Accumulator -= FixedDeltaSeconds;
        ++Steps;
    }

    if (Steps == MaxSubsteps && Accumulator >= FixedDeltaSeconds)
    {
        Accumulator = FMath::Fmod(Accumulator, static_cast<double>(FixedDeltaSeconds));
    }

    SynchronizeDynamicBodies();

    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    if (Settings && Settings->bEnableDebugDraw)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
            {
                if (UMossPhysicsBodyComponent* Component = Entry.Get())
                {
                    Component->DrawDebugBody(World, Settings->DebugDrawDuration);
                }
            }
        }
    }
}

TStatId UMossPhysicsWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UMossPhysicsWorldSubsystem, STATGROUP_Tickables);
}

void UMossPhysicsWorldSubsystem::HandleActorSpawned(AActor* Actor)
{
    if (!FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        return;
    }

    if (!PhysicsScene || !PhysicsScene->IsInitialized())
    {
        // The initial world scan will include this actor once the world is ready.
        bHasScannedActorTags = false;
        return;
    }

    RegisterActorFromTags(Actor);
}

bool UMossPhysicsWorldSubsystem::RegisterActorFromTags(AActor* Actor)
{
    const UWorld* World = GetWorld();
    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    if (!World || !World->IsGameWorld() || !Settings || !Settings->bEnableActorTagDiscovery || !IsValid(Actor) || Actor->IsTemplate())
    {
        return false;
    }

    const bool bStaticTagged = Actor->ActorHasTag(Settings->StaticActorTag);
    const bool bDynamicTagged = Actor->ActorHasTag(Settings->DynamicActorTag);
    if (!bStaticTagged && !bDynamicTagged)
    {
        return false;
    }
    if (bStaticTagged && bDynamicTagged)
    {
        FMossDiagnostics::Warning(FString::Printf(
            TEXT("Actor '%s' has both Moss static and dynamic tags. Remove one tag; no body was created."),
            *GetNameSafe(Actor)));
        return false;
    }

    if (Actor->FindComponentByClass<UMossPhysicsBodyComponent>())
    {
        return false;
    }

    USceneComponent* PreviousRoot = Actor->GetRootComponent();
    const FTransform PreviousRootTransform = PreviousRoot
        ? PreviousRoot->GetComponentTransform()
        : Actor->GetActorTransform();

    UMossPhysicsBodyComponent* BodyComponent = NewObject<UMossPhysicsBodyComponent>(Actor, NAME_None, RF_Transactional);
    if (!BodyComponent)
    {
        FMossDiagnostics::Warning(FString::Printf(TEXT("Failed to create a Moss body component for actor '%s'."), *GetNameSafe(Actor)));
        return false;
    }

    Actor->AddInstanceComponent(BodyComponent);
    BodyComponent->BodyMode = bStaticTagged ? EMossBodyMode::Static : EMossBodyMode::Dynamic;
    BodyComponent->CollisionLayer = bStaticTagged ? Settings->StaticActorCollisionLayer : Settings->DynamicActorCollisionLayer;
    BodyComponent->SetMobility(bDynamicTagged ? EComponentMobility::Movable : EComponentMobility::Static);
    BodyComponent->SetWorldTransform(PreviousRootTransform, false, nullptr, ETeleportType::TeleportPhysics);

    // Use the Moss body as the actor root so dynamic Moss transforms move the
    // complete actor hierarchy, not just a child adapter component.
    if (PreviousRoot)
    {
        if (bDynamicTagged)
        {
            if (PreviousRoot->Mobility != EComponentMobility::Movable)
            {
                PreviousRoot->SetMobility(EComponentMobility::Movable);
            }

            TArray<USceneComponent*> SceneComponents;
            Actor->GetComponents<USceneComponent>(SceneComponents, true);
            for (USceneComponent* SceneComponent : SceneComponents)
            {
                if (IsValid(SceneComponent) && SceneComponent->Mobility != EComponentMobility::Movable)
                {
                    SceneComponent->SetMobility(EComponentMobility::Movable);
                }
            }
        }
        Actor->SetRootComponent(BodyComponent);
        BodyComponent->RegisterComponent();
        PreviousRoot->AttachToComponent(BodyComponent, FAttachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        Actor->SetRootComponent(BodyComponent);
        BodyComponent->RegisterComponent();
    }

    return true;
}

int32 UMossPhysicsWorldSubsystem::RefreshTaggedActors()
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld() || !FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        return 0;
    }

    FString InitError;
    if (!EnsureNativeScene(InitError))
    {
        if (!InitError.IsEmpty()) FMossDiagnostics::Warning(InitError);
        return 0;
    }

    int32 RegisteredCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (RegisterActorFromTags(*It))
        {
            ++RegisteredCount;
        }
    }
    return RegisteredCount;
}

bool UMossPhysicsWorldSubsystem::IsTickable() const
{
    const UWorld* World = GetWorld();
    return !IsTemplate() && World && World->IsGameWorld() && FMossPhysicsScene::IsNativePhysicsCompiled();
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
        OutError = TEXT("Cannot register a Moss body because its component is invalid.");
        return false;
    }

    RegisteredBodies.AddUnique(TWeakObjectPtr<UMossPhysicsBodyComponent>(Component));
    if (!FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        RegisteredBodies.RemoveAll([Component](const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry)
        {
            return !Entry.IsValid() || Entry.Get() == Component;
        });
        OutError = TEXT("Moss Physics is currently compile-gated; no native body was created.");
        return false;
    }

    if (!PhysicsScene || !PhysicsScene->IsInitialized())
    {
        if (!EnsureNativeScene(OutError))
        {
            // Keep the accepted registration queued while a transient world/config
            // initialization issue is being retried. The component retains exclusive
            // Moss ownership of its collision primitives during this pending period.
            bPendingBodiesNeedImmediateRetry = true;
            OutError.Reset();
            return true;
        }
    }

    if (!CreateBodyForComponent(Component, OutError))
    {
        RegisteredBodies.RemoveAll([Component](const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry)
        {
            return !Entry.IsValid() || Entry.Get() == Component;
        });
        return false;
    }
    return true;
}

bool UMossPhysicsWorldSubsystem::CreateBodyForComponent(UMossPhysicsBodyComponent* Component, FString& OutError)
{
    if (!IsValid(Component) || !PhysicsScene || !PhysicsScene->IsInitialized())
    {
        OutError = TEXT("Cannot create a Moss body until its owning world has an initialized Moss PhysicsSystem.");
        return false;
    }
    if (Component->NativeBody.IsValid())
    {
        OutError.Reset();
        return true;
    }

    FMossPhysicsBodyDesc Description;
    if (!Component->BuildNativeDescription(Description, OutError))
    {
        return false;
    }

    const uint32 BodyId = PhysicsScene->CreateBody(Description, OutError);
    if (BodyId == FMossPhysicsBodyId::InvalidValue)
    {
        return false;
    }

    Component->SetNativeBodyId(BodyId);
    BodyComponentsById.Add(BodyId, Component);
    OutError.Reset();
    return true;
}

void UMossPhysicsWorldSubsystem::TryCreatePendingBodies()
{
    // Iterate backwards so failures can be removed without invalidating an active
    // TArray range-for iterator. Preserve registration order for the remaining bodies.
    for (int32 Index = RegisteredBodies.Num() - 1; Index >= 0; --Index)
    {
        UMossPhysicsBodyComponent* Component = RegisteredBodies[Index].Get();
        if (!IsValid(Component))
        {
            RegisteredBodies.RemoveAt(Index, 1, EAllowShrinking::No);
            continue;
        }

        if (Component->NativeBody.IsValid())
        {
            continue;
        }

        FString Error;
        if (!CreateBodyForComponent(Component, Error))
        {
            RegisteredBodies.RemoveAt(Index, 1, EAllowShrinking::No);
            Component->RestoreChaosOwnership();
            if (!Error.IsEmpty())
            {
                FMossDiagnostics::Warning(Error);
            }
        }
    }
}

void UMossPhysicsWorldSubsystem::UnregisterBody(UMossPhysicsBodyComponent* Component)
{
    if (!Component)
    {
        return;
    }

    const FMossPhysicsBodyId Body = Component->NativeBody;
    Component->NativeBody.Reset();
    if (Body.IsValid())
    {
        BodyComponentsById.Remove(Body.Value);
    }
    if (PhysicsScene && PhysicsScene->IsInitialized() && Body.IsValid())
    {
        PhysicsScene->DestroyBody(Body.Value);
    }

    RegisteredBodies.RemoveAll([Component](const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry)
    {
        return !Entry.IsValid() || Entry.Get() == Component;
    });
}

bool UMossPhysicsWorldSubsystem::Raycast(const FVector& Start, const FVector& End, FMossPhysicsRaycastHit& OutHit)
{
    OutHit = FMossPhysicsRaycastHit();
    if (!PhysicsScene || !PhysicsScene->IsInitialized())
    {
        FMossDiagnostics::Warning(TEXT("Moss raycast requested before the Moss PhysicsSystem was initialized."));
        return false;
    }

    FString Error;
    const bool bSucceeded = PhysicsScene->Raycast(Start, End, OutHit, Error);
    if (!Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
    return bSucceeded && OutHit.bBlockingHit;
}

void UMossPhysicsWorldSubsystem::SynchronizeKinematicBodies()
{
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            if (Component->GetBodyMode() == EMossBodyMode::Kinematic)
            {
                Component->SynchronizeToNative(this, FixedDeltaSeconds);
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

void UMossPhysicsWorldSubsystem::DispatchContactEvents()
{
    if (!PhysicsScene)
    {
        return;
    }

    FMossPhysicsContactEventNative NativeEvent;
    while (PhysicsScene->PopContactEvent(NativeEvent))
    {
        FMossPhysicsContact Contact;
        Contact.BodyA = NativeEvent.BodyA;
        Contact.BodyB = NativeEvent.BodyB;
        Contact.Point = NativeEvent.PointCm;
        Contact.Normal = NativeEvent.Normal;
        Contact.PenetrationDepth = NativeEvent.PenetrationDepthCm;
        OnMossContactAdded.Broadcast(Contact);

        const auto BroadcastToBody = [this, &Contact](const uint32 BodyId)
        {
            if (const TWeakObjectPtr<UMossPhysicsBodyComponent>* ComponentEntry = BodyComponentsById.Find(BodyId))
            {
                if (UMossPhysicsBodyComponent* Component = ComponentEntry->Get())
                {
                    Component->OnMossContactAdded.Broadcast(Contact);
                }
                else
                {
                    BodyComponentsById.Remove(BodyId);
                }
            }
        };
        BroadcastToBody(NativeEvent.BodyA);
        if (NativeEvent.BodyB != NativeEvent.BodyA)
        {
            BroadcastToBody(NativeEvent.BodyB);
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
            const FMossPhysicsBodyId Body = Component->NativeBody;
            Component->NativeBody.Reset();
            if (PhysicsScene && PhysicsScene->IsInitialized() && Body.IsValid())
            {
                PhysicsScene->DestroyBody(Body.Value);
            }
            Component->RestoreChaosOwnership();
        }
    }
    RegisteredBodies.Reset();
    BodyComponentsById.Reset();
    PendingBodyRetryAccumulator = 0.0;
    bPendingBodiesNeedImmediateRetry = false;
}

bool UMossPhysicsWorldSubsystem::WakeBody(FMossPhysicsBodyId Body, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->WakeBody(Body.Value, OutError);
}

bool UMossPhysicsWorldSubsystem::ApplyForce(FMossPhysicsBodyId Body, const FVector& ForceNewtons, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->AddForce(Body.Value, ForceNewtons, OutError);
}

bool UMossPhysicsWorldSubsystem::ApplyImpulse(FMossPhysicsBodyId Body, const FVector& ImpulseNewtonSeconds, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->AddImpulse(Body.Value, ImpulseNewtonSeconds, OutError);
}

bool UMossPhysicsWorldSubsystem::ApplyTorque(FMossPhysicsBodyId Body, const FVector& TorqueNewtonMeters, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->AddTorque(Body.Value, TorqueNewtonMeters, OutError);
}

bool UMossPhysicsWorldSubsystem::SetLinearVelocity(FMossPhysicsBodyId Body, const FVector& VelocityCmPerSecond, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetLinearVelocity(Body.Value, VelocityCmPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::GetLinearVelocity(FMossPhysicsBodyId Body, FVector& OutVelocityCmPerSecond, FString& OutError) const
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->GetLinearVelocity(Body.Value, OutVelocityCmPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::SetAngularVelocity(FMossPhysicsBodyId Body, const FVector& AngularVelocityRadiansPerSecond, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetAngularVelocity(Body.Value, AngularVelocityRadiansPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::GetAngularVelocity(FMossPhysicsBodyId Body, FVector& OutAngularVelocityRadiansPerSecond, FString& OutError) const
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->GetAngularVelocity(Body.Value, OutAngularVelocityRadiansPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::SetFriction(FMossPhysicsBodyId Body, float Friction, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetFriction(Body.Value, Friction, OutError);
}

bool UMossPhysicsWorldSubsystem::SetRestitution(FMossPhysicsBodyId Body, float Restitution, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetRestitution(Body.Value, Restitution, OutError);
}

bool UMossPhysicsWorldSubsystem::SetGravityFactor(FMossPhysicsBodyId Body, float GravityFactor, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetGravityFactor(Body.Value, GravityFactor, OutError);
}

bool UMossPhysicsWorldSubsystem::SetMass(FMossPhysicsBodyId Body, float MassKg, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetMass(Body.Value, MassKg, OutError);
}

bool UMossPhysicsWorldSubsystem::SetLinearDamping(FMossPhysicsBodyId Body, float Damping, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetLinearDamping(Body.Value, Damping, OutError);
}

bool UMossPhysicsWorldSubsystem::SetAngularDamping(FMossPhysicsBodyId Body, float Damping, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetAngularDamping(Body.Value, Damping, OutError);
}

bool UMossPhysicsWorldSubsystem::SetMaxLinearVelocity(FMossPhysicsBodyId Body, float MetersPerSecond, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetMaxLinearVelocity(Body.Value, MetersPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::SetMaxAngularVelocity(FMossPhysicsBodyId Body, float RadiansPerSecond, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetMaxAngularVelocity(Body.Value, RadiansPerSecond, OutError);
}

bool UMossPhysicsWorldSubsystem::SetAllowSleeping(FMossPhysicsBodyId Body, bool bAllowSleeping, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetAllowSleeping(Body.Value, bAllowSleeping, OutError);
}

bool UMossPhysicsWorldSubsystem::SetMotionQuality(FMossPhysicsBodyId Body, bool bLinearCast, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetMotionQuality(Body.Value, bLinearCast, OutError);
}

bool UMossPhysicsWorldSubsystem::SetEnhancedInternalEdgeRemoval(FMossPhysicsBodyId Body, bool bEnabled, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetEnhancedInternalEdgeRemoval(Body.Value, bEnabled, OutError);
}

bool UMossPhysicsWorldSubsystem::SetApplyGyroscopicForce(FMossPhysicsBodyId Body, bool bEnabled, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetApplyGyroscopicForce(Body.Value, bEnabled, OutError);
}

bool UMossPhysicsWorldSubsystem::SetSolverSteps(FMossPhysicsBodyId Body, uint32 VelocitySteps, uint32 PositionSteps, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetSolverSteps(Body.Value, VelocitySteps, PositionSteps, OutError);
}

bool UMossPhysicsWorldSubsystem::SetSensor(FMossPhysicsBodyId Body, bool bSensor, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetSensor(Body.Value, bSensor, OutError);
}

bool UMossPhysicsWorldSubsystem::SetObjectLayer(FMossPhysicsBodyId Body, int32 ObjectLayer, FString& OutError)
{
    return PhysicsScene && Body.IsValid() && PhysicsScene->SetObjectLayer(Body.Value, ObjectLayer, OutError);
}

void UMossPhysicsWorldSubsystem::InvalidateBodyHandlesPreserveRegistrations()
{
    // A failed native update invalidates every BodyID owned by the old
    // PhysicsSystem, but the Unreal components remain registered so a newly
    // initialized world can recreate their Moss bodies.
    for (const TWeakObjectPtr<UMossPhysicsBodyComponent>& Entry : RegisteredBodies)
    {
        if (UMossPhysicsBodyComponent* Component = Entry.Get())
        {
            Component->NativeBody.Reset();
        }
    }
    BodyComponentsById.Reset();
    PendingBodyRetryAccumulator = 0.0;
    bPendingBodiesNeedImmediateRetry = true;
}
