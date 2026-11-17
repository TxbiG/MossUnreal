#include "MossPhysicsBodyComponent.h"

#include "MossPhysicsWorldSubsystem.h"
#include "Core/MossRuntime.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Diagnostics/MossDiagnostics.h"

UMossPhysicsBodyComponent::UMossPhysicsBodyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetMobility(EComponentMobility::Movable);
}

void UMossPhysicsBodyComponent::BeginPlay()
{
    Super::BeginPlay();
    RecreateMossBody();
}

void UMossPhysicsBodyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        if (UMossPhysicsWorldSubsystem* MossWorld = World->GetSubsystem<UMossPhysicsWorldSubsystem>())
        {
            MossWorld->UnregisterBody(this);
        }
    }
    NativeBody.Reset();
    Super::EndPlay(EndPlayReason);
}

bool UMossPhysicsBodyComponent::RecreateMossBody()
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* MossWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    if (!MossWorld)
    {
        FMossDiagnostics::Warning(TEXT("Cannot create Moss body because this component has no valid Moss world subsystem."));
        return false;
    }

    MossWorld->UnregisterBody(this);
    FString Error;
    if (!MossWorld->RegisterBody(this, Error))
    {
        FMossDiagnostics::Warning(Error);
        return false;
    }
    return true;
}

FMossNativeBodyDescription UMossPhysicsBodyComponent::BuildNativeDescription() const
{
    FMossNativeBodyDescription Description;
    Description.Mode = BodyMode == EMossBodyMode::Static
        ? EMossNativeBodyMode::Static
        : (BodyMode == EMossBodyMode::Kinematic ? EMossNativeBodyMode::Kinematic : EMossNativeBodyMode::Dynamic);
    Description.Shape = CollisionShape == EMossCollisionShape::Sphere
        ? EMossNativeShape::Sphere
        : (CollisionShape == EMossCollisionShape::Capsule ? EMossNativeShape::Capsule : EMossNativeShape::Box);
    const FVector WorldScale = GetComponentScale().GetAbs();
    Description.InitialTransform = FTransform(GetComponentQuat(), GetComponentLocation(), FVector::OneVector);
    Description.BoxHalfExtents = BoxHalfExtents.GetAbs() * WorldScale;
    const double RadialScale = FMath::Max(WorldScale.X, WorldScale.Y);
    const double SphereScale = FMath::Max(WorldScale.X, FMath::Max(WorldScale.Y, WorldScale.Z));
    Description.Radius = FMath::Max(0.1f, Radius) *
        (CollisionShape == EMossCollisionShape::Sphere ? SphereScale : RadialScale);
    Description.HalfHeight = FMath::Max(0.1f, HalfHeight) * WorldScale.Z;
    Description.HalfHeight = FMath::Max(Description.HalfHeight, Description.Radius);
    Description.Mass = FMath::Max(0.001f, Mass);
    Description.LinearDamping = FMath::Max(0.0f, LinearDamping);
    Description.AngularDamping = FMath::Max(0.0f, AngularDamping);
    Description.bUseGravity = bUseGravity;
    return Description;
}

void UMossPhysicsBodyComponent::SynchronizeToNative(UMossPhysicsWorldSubsystem* WorldSubsystem)
{
    if (!bSynchronizeTransform || !WorldSubsystem || !NativeBody.IsValid())
    {
        return;
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (Backend.IsValid())
    {
        FString Error;
        const FTransform PhysicsTransform(GetComponentQuat(), GetComponentLocation(), FVector::OneVector);
        if (!Backend->SetBodyTransform(WorldSubsystem->NativeWorld, NativeBody, PhysicsTransform, Error) && !Error.IsEmpty())
        {
            FMossDiagnostics::Warning(Error);
        }
    }
}

void UMossPhysicsBodyComponent::SynchronizeFromNative(UMossPhysicsWorldSubsystem* WorldSubsystem)
{
    if (!bSynchronizeTransform || !WorldSubsystem || !NativeBody.IsValid())
    {
        return;
    }

    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend = FMossRuntime::Get().GetBackend();
    if (!Backend.IsValid())
    {
        return;
    }

    FTransform NativeTransform;
    FString Error;
    if (Backend->GetBodyTransform(WorldSubsystem->NativeWorld, NativeBody, NativeTransform, Error))
    {
        NativeTransform.SetScale3D(GetComponentScale());
        SetWorldTransform(NativeTransform, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else if (!Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
}
