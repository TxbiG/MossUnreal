#include "Physics/MossPhysicsBodyComponent.h"

#include "Physics/MossPhysicsWorldSubsystem.h"
#include "Physics/MossPhysicsScene.h"
#include "Physics/MossPhysicsSettings.h"
#include "Physics/MossPhysicsUnits.h"
#include "Diagnostics/MossDiagnostics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "DrawDebugHelpers.h"

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
        if (UMossPhysicsWorldSubsystem* PhysicsWorld = World->GetSubsystem<UMossPhysicsWorldSubsystem>())
        {
            PhysicsWorld->UnregisterBody(this);
        }
    }
    NativeBody.Reset();
    RestoreChaosOwnership();
    Super::EndPlay(EndPlayReason);
}

bool UMossPhysicsBodyComponent::RecreateMossBody()
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    if (!PhysicsWorld)
    {
        RestoreChaosOwnership();
        FMossDiagnostics::Warning(TEXT("Cannot create a Moss body without a valid UWorld Moss physics subsystem."));
        return false;
    }

    // Retire the old Moss body before restoring Chaos state or attempting a
    // new ownership transition. This avoids overlapping native body lifetimes.
    PhysicsWorld->UnregisterBody(this);
    RestoreChaosOwnership();

    FString Error;
    if (!ResolveChaosOwnership(Error))
    {
        if (!Error.IsEmpty()) FMossDiagnostics::Warning(Error);
        return false;
    }

    if (!PhysicsWorld->RegisterBody(this, Error))
    {
        RestoreChaosOwnership();
        if (!Error.IsEmpty())
        {
            FMossDiagnostics::Warning(Error);
        }
        return false;
    }
    return IsMossBodyReady();
}


bool UMossPhysicsBodyComponent::ResolveChaosOwnership(FString& OutError)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor || !FMossPhysicsScene::IsNativePhysicsCompiled())
    {
        OutError.Reset();
        return true;
    }

    TArray<UPrimitiveComponent*> PrimitiveComponents;
    OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents, true);

    bool bHasChaosPhysicsOwnership = false;
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (!IsValid(Primitive))
        {
            continue;
        }

        const ECollisionEnabled::Type PreviousCollision = Primitive->GetCollisionEnabled();
        const bool bWasSimulating = Primitive->IsSimulatingPhysics();
        const bool bParticipatesInChaosPhysics = bWasSimulating
            || PreviousCollision == ECollisionEnabled::QueryAndPhysics
            || PreviousCollision == ECollisionEnabled::PhysicsOnly;
        bHasChaosPhysicsOwnership |= bParticipatesInChaosPhysics;
    }

    if (bHasChaosPhysicsOwnership && ChaosConflictPolicy == EMossChaosConflictPolicy::RejectMossBody)
    {
        OutError = FString::Printf(
            TEXT("Moss body '%s' was not registered because one or more owner primitive components still participate in Chaos physics. Disable their physics collision or select 'Disable Chaos Physics Collision on Owner'."),
            *GetNameSafe(this));
        return false;
    }

    // Moss-owned primitives must not remain as physical Chaos shapes. QueryOnly
    // preserves Unreal traces/overlaps for QueryAndPhysics components. PhysicsOnly
    // becomes NoCollision so we do not silently add query behavior that was absent.
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (!IsValid(Primitive))
        {
            continue;
        }

        const ECollisionEnabled::Type PreviousCollision = Primitive->GetCollisionEnabled();
        const bool bWasSimulating = Primitive->IsSimulatingPhysics();
        if (!bWasSimulating
            && PreviousCollision != ECollisionEnabled::QueryAndPhysics
            && PreviousCollision != ECollisionEnabled::PhysicsOnly)
        {
            continue;
        }

        FMossChaosPrimitiveState& State = PreviousChaosPrimitiveStates.AddDefaulted_GetRef();
        State.Primitive = Primitive;
        State.PreviousCollisionEnabled = PreviousCollision;
        State.bWasSimulatingPhysics = bWasSimulating;

        if (bWasSimulating)
        {
            Primitive->SetSimulatePhysics(false);
        }

        Primitive->SetCollisionEnabled(PreviousCollision == ECollisionEnabled::QueryAndPhysics
            ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision);
    }

    OutError.Reset();
    return true;
}

void UMossPhysicsBodyComponent::RestoreChaosOwnership()
{
    for (const FMossChaosPrimitiveState& State : PreviousChaosPrimitiveStates)
    {
        UPrimitiveComponent* Primitive = State.Primitive.Get();
        if (!IsValid(Primitive))
        {
            continue;
        }

        if (State.bWasSimulatingPhysics)
        {
            Primitive->SetSimulatePhysics(true);
        }
        Primitive->SetCollisionEnabled(State.PreviousCollisionEnabled);
    }

    PreviousChaosPrimitiveStates.Reset();
}

bool UMossPhysicsBodyComponent::BuildNativeDescription(FMossPhysicsBodyDesc& OutDescription, FString& OutError) const
{
    OutDescription = FMossPhysicsBodyDesc();
    OutDescription.MotionType = BodyMode;
    OutDescription.WorldTransform = FTransform(GetComponentQuat(), GetComponentLocation(), FVector::OneVector);
    OutDescription.MassKg = FMath::Max(0.001f, MassKg);
    OutDescription.Friction = FMath::Max(0.0f, Friction);
    OutDescription.Restitution = FMath::Clamp(Restitution, 0.0f, 1.0f);
    OutDescription.LinearDamping = FMath::Max(0.0f, LinearDamping);
    OutDescription.AngularDamping = FMath::Max(0.0f, AngularDamping);
    OutDescription.GravityFactor = GravityFactor;
    OutDescription.MaxLinearVelocityMps = FMath::Max(0.0f, MaxLinearVelocityMetersPerSecond);
    OutDescription.MaxAngularVelocityRadians = FMath::Max(0.0f, MaxAngularVelocityRadiansPerSecond);
    OutDescription.AllowedDOFs = static_cast<uint32>(AllowedDOFs) & 0x3fu;
    OutDescription.VelocitySolverSteps = static_cast<uint32>(FMath::Clamp(VelocitySolverSteps, 0, 255));
    OutDescription.PositionSolverSteps = static_cast<uint32>(FMath::Clamp(PositionSolverSteps, 0, 255));
    OutDescription.bSensor = bSensor;
    OutDescription.bAllowSleeping = bAllowSleeping;
    OutDescription.bApplyGyroscopicForce = bApplyGyroscopicForce;
    OutDescription.bEnhancedInternalEdgeRemoval = bEnhancedInternalEdgeRemoval;
    OutDescription.bLinearCastMotionQuality = MotionQuality == EMossPhysicsMotionQuality::LinearCast;

    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    OutDescription.ObjectLayer = Settings ? Settings->FindObjectLayer(CollisionLayer) : 0;

    bool bAddedShape = false;
    if (bGatherOwnerCollisionComponents)
    {
        if (AActor* OwnerActor = GetOwner())
        {
            TArray<UShapeComponent*> ShapeComponents;
            OwnerActor->GetComponents<UShapeComponent>(ShapeComponents, true);
            for (UShapeComponent* ShapeComponent : ShapeComponents)
            {
                if (!IsValid(ShapeComponent) || ShapeComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
                {
                    continue;
                }

                FMossPhysicsPrimitiveDesc Primitive;
                const FVector LocalPosition = GetComponentTransform().InverseTransformPositionNoScale(ShapeComponent->GetComponentLocation());
                Primitive.LocalPositionCm = LocalPosition;
                Primitive.LocalRotation = GetComponentQuat().Inverse() * ShapeComponent->GetComponentQuat();

                if (const UBoxComponent* Box = Cast<UBoxComponent>(ShapeComponent))
                {
                    Primitive.Shape = EMossCollisionShape::Box;
                    Primitive.BoxHalfExtentsCm = Box->GetScaledBoxExtent().GetAbs();
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }
                else if (const USphereComponent* Sphere = Cast<USphereComponent>(ShapeComponent))
                {
                    Primitive.Shape = EMossCollisionShape::Sphere;
                    Primitive.RadiusCm = Sphere->GetScaledSphereRadius();
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }
                else if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(ShapeComponent))
                {
                    Primitive.Shape = EMossCollisionShape::Capsule;
                    float ScaledRadius = 0.0f;
                    float ScaledHalfHeight = 0.0f;
                    Capsule->GetScaledCapsuleSize(ScaledRadius, ScaledHalfHeight);
                    Primitive.RadiusCm = ScaledRadius;
                    Primitive.CapsuleHalfHeightCm = ScaledHalfHeight;
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }
            }

            // Static meshes contribute their authored simple collision geometry as
            // one compound body. Complex triangle meshes are deliberately not
            // triangulated here; they will need a dedicated mesh-shape path.
            TArray<UStaticMeshComponent*> MeshComponents;
            OwnerActor->GetComponents<UStaticMeshComponent>(MeshComponents, true);
            for (const UStaticMeshComponent* MeshComponent : MeshComponents)
            {
                if (!IsValid(MeshComponent) || MeshComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
                {
                    continue;
                }
                const UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
                const UBodySetup* BodySetup = StaticMesh ? StaticMesh->GetBodySetup() : nullptr;
                if (!BodySetup)
                {
                    continue;
                }

                const FTransform MeshTransform = MeshComponent->GetComponentTransform();
                const FVector MeshScale = MeshComponent->GetComponentScale().GetAbs();
                const FQuat RootInverse = GetComponentQuat().Inverse();
                const FKAggregateGeom& Geometry = BodySetup->AggGeom;

                for (const FKBoxElem& Box : Geometry.BoxElems)
                {
                    FMossPhysicsPrimitiveDesc Primitive;
                    Primitive.Shape = EMossCollisionShape::Box;
                    Primitive.LocalPositionCm = GetComponentTransform().InverseTransformPositionNoScale(MeshTransform.TransformPosition(Box.Center));
                    Primitive.LocalRotation = RootInverse * MeshComponent->GetComponentQuat() * Box.Rotation.Quaternion();
                    Primitive.BoxHalfExtentsCm = FVector(Box.X * MeshScale.X, Box.Y * MeshScale.Y, Box.Z * MeshScale.Z).GetAbs();
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }

                for (const FKSphereElem& Sphere : Geometry.SphereElems)
                {
                    FMossPhysicsPrimitiveDesc Primitive;
                    Primitive.Shape = EMossCollisionShape::Sphere;
                    Primitive.LocalPositionCm = GetComponentTransform().InverseTransformPositionNoScale(MeshTransform.TransformPosition(Sphere.Center));
                    Primitive.LocalRotation = RootInverse * MeshComponent->GetComponentQuat();
                    Primitive.RadiusCm = Sphere.Radius * FMath::Max3(MeshScale.X, MeshScale.Y, MeshScale.Z);
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }

                for (const FKSphylElem& Sphyl : Geometry.SphylElems)
                {
                    FMossPhysicsPrimitiveDesc Primitive;
                    Primitive.Shape = EMossCollisionShape::Capsule;
                    Primitive.LocalPositionCm = GetComponentTransform().InverseTransformPositionNoScale(MeshTransform.TransformPosition(Sphyl.Center));
                    Primitive.LocalRotation = RootInverse * MeshComponent->GetComponentQuat() * Sphyl.Rotation.Quaternion();
                    const float RadialScale = FMath::Max(MeshScale.X, MeshScale.Y);
                    Primitive.RadiusCm = Sphyl.Radius * RadialScale;
                    Primitive.CapsuleHalfHeightCm = Primitive.RadiusCm + Sphyl.Length * MeshScale.Z * 0.5f;
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }

                for (const FKConvexElem& Convex : Geometry.ConvexElems)
                {
                    if (Convex.VertexData.Num() < 4)
                    {
                        continue;
                    }
                    FMossPhysicsPrimitiveDesc Primitive;
                    Primitive.Shape = EMossCollisionShape::ConvexHull;
                    Primitive.ConvexVerticesCm.Reserve(Convex.VertexData.Num());
                    for (const FVector& Vertex : Convex.VertexData)
                    {
                        const FVector WorldVertex = MeshTransform.TransformPosition(Vertex);
                        Primitive.ConvexVerticesCm.Add(GetComponentTransform().InverseTransformPositionNoScale(WorldVertex));
                    }
                    // Vertices are transformed into the body component's local
                    // frame, so the convex subshape itself remains at identity.
                    Primitive.LocalPositionCm = FVector::ZeroVector;
                    Primitive.LocalRotation = FQuat::Identity;
                    OutDescription.Primitives.Add(MoveTemp(Primitive));
                    bAddedShape = true;
                }
            }
        }
    }

    if (!bAddedShape)
    {
        FMossPhysicsPrimitiveDesc Primitive;
        Primitive.Shape = FallbackShape;
        const FVector Scale = GetComponentScale().GetAbs();
        Primitive.BoxHalfExtentsCm = BoxHalfExtents.GetAbs() * Scale;
        const float RadialScale = FMath::Max(Scale.X, Scale.Y);
        const float UniformScale = FMath::Max3(Scale.X, Scale.Y, Scale.Z);
        Primitive.RadiusCm = FMath::Max(0.1f, Radius) * (FallbackShape == EMossCollisionShape::Sphere ? UniformScale : RadialScale);
        Primitive.CapsuleHalfHeightCm = FMath::Max(Primitive.RadiusCm, CapsuleHalfHeight * Scale.Z);
        Primitive.ConvexVerticesCm.Reserve(ConvexHullVertices.Num());
        for (const FVector& Vertex : ConvexHullVertices)
        {
            Primitive.ConvexVerticesCm.Add(Vertex * Scale);
        }
        OutDescription.Primitives.Add(MoveTemp(Primitive));
    }

    if (OutDescription.Primitives.IsEmpty())
    {
        OutError = TEXT("Moss body has no supported collision primitives. Add Box, Sphere or Capsule components, supported Static Mesh simple collision, or configure the fallback primitive.");
        return false;
    }

    if (BodyMode == EMossBodyMode::Dynamic && OutDescription.AllowedDOFs == 0)
    {
        OutError = TEXT("A dynamic Moss body must allow at least one degree of freedom. Use a static body for a fully locked object.");
        return false;
    }

    OutError.Reset();
    return true;
}

bool UMossPhysicsBodyComponent::WakeBody()
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->WakeBody(NativeBody, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::AddForce(const FVector& ForceNewtons)
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->ApplyForce(NativeBody, ForceNewtons, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::AddImpulse(const FVector& ImpulseNewtonSeconds)
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->ApplyImpulse(NativeBody, ImpulseNewtonSeconds, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::AddTorque(const FVector& TorqueNewtonMeters)
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->ApplyTorque(NativeBody, TorqueNewtonMeters, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetLinearVelocity(const FVector& VelocityCentimetersPerSecond)
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetLinearVelocity(NativeBody, VelocityCentimetersPerSecond, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

FVector UMossPhysicsBodyComponent::GetLinearVelocity() const
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FVector Velocity = FVector::ZeroVector;
    FString Error;
    if (!PhysicsWorld || !PhysicsWorld->GetLinearVelocity(NativeBody, Velocity, Error))
    {
        if (!Error.IsEmpty()) FMossDiagnostics::Warning(Error);
        return FVector::ZeroVector;
    }
    return Velocity;
}

bool UMossPhysicsBodyComponent::SetAngularVelocity(const FVector& AngularVelocityRadiansPerSecond)
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetAngularVelocity(NativeBody, AngularVelocityRadiansPerSecond, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

FVector UMossPhysicsBodyComponent::GetAngularVelocity() const
{
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FVector Velocity = FVector::ZeroVector;
    FString Error;
    if (!PhysicsWorld || !PhysicsWorld->GetAngularVelocity(NativeBody, Velocity, Error))
    {
        if (!Error.IsEmpty()) FMossDiagnostics::Warning(Error);
        return FVector::ZeroVector;
    }
    return Velocity;
}

bool UMossPhysicsBodyComponent::SetFriction(const float NewFriction)
{
    Friction = FMath::Max(0.0f, NewFriction);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetFriction(NativeBody, Friction, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetRestitution(const float NewRestitution)
{
    Restitution = FMath::Clamp(NewRestitution, 0.0f, 1.0f);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetRestitution(NativeBody, Restitution, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetGravityFactor(const float NewGravityFactor)
{
    GravityFactor = NewGravityFactor;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetGravityFactor(NativeBody, GravityFactor, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetMass(float NewMassKg)
{
    MassKg = FMath::Max(0.001f, NewMassKg);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetMass(NativeBody, MassKg, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetLinearDamping(float NewDamping)
{
    LinearDamping = FMath::Max(0.0f, NewDamping);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetLinearDamping(NativeBody, LinearDamping, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetAngularDamping(float NewDamping)
{
    AngularDamping = FMath::Max(0.0f, NewDamping);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetAngularDamping(NativeBody, AngularDamping, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetMaxLinearVelocity(float MetersPerSecond)
{
    MaxLinearVelocityMetersPerSecond = FMath::Max(0.0f, MetersPerSecond);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetMaxLinearVelocity(NativeBody, MaxLinearVelocityMetersPerSecond, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetMaxAngularVelocity(float RadiansPerSecond)
{
    MaxAngularVelocityRadiansPerSecond = FMath::Max(0.0f, RadiansPerSecond);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetMaxAngularVelocity(NativeBody, MaxAngularVelocityRadiansPerSecond, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetAllowSleeping(bool bNewAllowSleeping)
{
    bAllowSleeping = bNewAllowSleeping;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetAllowSleeping(NativeBody, bAllowSleeping, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetMotionQuality(EMossPhysicsMotionQuality NewMotionQuality)
{
    MotionQuality = NewMotionQuality;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetMotionQuality(NativeBody, MotionQuality == EMossPhysicsMotionQuality::LinearCast, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetEnhancedInternalEdgeRemoval(bool bEnabled)
{
    bEnhancedInternalEdgeRemoval = bEnabled;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetEnhancedInternalEdgeRemoval(NativeBody, bEnabled, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetApplyGyroscopicForce(bool bEnabled)
{
    bApplyGyroscopicForce = bEnabled;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetApplyGyroscopicForce(NativeBody, bEnabled, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetSolverSteps(int32 NewVelocitySteps, int32 NewPositionSteps)
{
    VelocitySolverSteps = FMath::Clamp(NewVelocitySteps, 0, 255);
    PositionSolverSteps = FMath::Clamp(NewPositionSteps, 0, 255);
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetSolverSteps(
        NativeBody, static_cast<uint32>(VelocitySolverSteps), static_cast<uint32>(PositionSolverSteps), Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetSensor(bool bNewSensor)
{
    bSensor = bNewSensor;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetSensor(NativeBody, bSensor, Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

bool UMossPhysicsBodyComponent::SetCollisionLayer(FName NewCollisionLayer)
{
    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    const bool bLayerExists = Settings && Settings->ObjectLayers.ContainsByPredicate(
        [NewCollisionLayer](const FMossPhysicsLayerDefinition& Layer) { return Layer.Name == NewCollisionLayer; });
    if (!bLayerExists)
    {
        FMossDiagnostics::Warning(FString::Printf(TEXT("Unknown Moss collision layer '%s'."), *NewCollisionLayer.ToString()));
        return false;
    }

    CollisionLayer = NewCollisionLayer;
    UWorld* World = GetWorld();
    UMossPhysicsWorldSubsystem* PhysicsWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    FString Error;
    const bool bSucceeded = PhysicsWorld && PhysicsWorld->SetObjectLayer(NativeBody, Settings->FindObjectLayer(CollisionLayer), Error);
    if (!bSucceeded && !Error.IsEmpty()) FMossDiagnostics::Warning(Error);
    return bSucceeded;
}

void UMossPhysicsBodyComponent::SynchronizeToNative(UMossPhysicsWorldSubsystem* WorldSubsystem, const float DeltaSeconds)
{
    if (!bSynchronizeTransform || !WorldSubsystem || !NativeBody.IsValid()) return;
    if (!WorldSubsystem->PhysicsScene) return;

    FString Error;
    const FTransform Transform(GetComponentQuat(), GetComponentLocation(), FVector::OneVector);
    if (!WorldSubsystem->PhysicsScene->SetBodyTransform(NativeBody.Value, Transform, BodyMode == EMossBodyMode::Kinematic, DeltaSeconds, Error) && !Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
}

void UMossPhysicsBodyComponent::SynchronizeFromNative(UMossPhysicsWorldSubsystem* WorldSubsystem)
{
    if (!bSynchronizeTransform || !WorldSubsystem || !NativeBody.IsValid() || !WorldSubsystem->PhysicsScene) return;

    FTransform NativeTransform;
    FString Error;
    if (WorldSubsystem->PhysicsScene->GetBodyTransform(NativeBody.Value, NativeTransform, Error))
    {
        NativeTransform.SetScale3D(GetComponentScale());
        SetWorldTransform(NativeTransform, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else if (!Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
}

void UMossPhysicsBodyComponent::DrawDebugBody(UWorld* World, const float Duration) const
{
    if (!World)
    {
        return;
    }

    const FColor Color(40, 190, 255);
    bool bDrewAny = false;
    if (bGatherOwnerCollisionComponents)
    {
        if (AActor* OwnerActor = GetOwner())
        {
            TArray<UShapeComponent*> ShapeComponents;
            OwnerActor->GetComponents<UShapeComponent>(ShapeComponents, true);
            for (const UShapeComponent* Shape : ShapeComponents)
            {
                if (!IsValid(Shape) || Shape->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
                {
                    continue;
                }
                if (const UBoxComponent* Box = Cast<UBoxComponent>(Shape))
                {
                    DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), Box->GetComponentQuat(), Color, false, Duration, 0, 1.0f);
                    bDrewAny = true;
                }
                else if (const USphereComponent* Sphere = Cast<USphereComponent>(Shape))
                {
                    DrawDebugSphere(World, Sphere->GetComponentLocation(), Sphere->GetScaledSphereRadius(), 16, Color, false, Duration, 0, 1.0f);
                    bDrewAny = true;
                }
                else if (const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Shape))
                {
                    float ScaledRadius = 0.0f;
                    float ScaledHalfHeight = 0.0f;
                    Capsule->GetScaledCapsuleSize(ScaledRadius, ScaledHalfHeight);
                    DrawDebugCapsule(World, Capsule->GetComponentLocation(), ScaledHalfHeight, ScaledRadius, Capsule->GetComponentQuat(), Color, false, Duration, 0, 1.0f);
                    bDrewAny = true;
                }
            }

            // Static-mesh simple collision is extracted from BodySetup::AggGeom.
            // Bounds are shown here as a visual placement aid, not as exact mesh geometry.
            TArray<UStaticMeshComponent*> MeshComponents;
            OwnerActor->GetComponents<UStaticMeshComponent>(MeshComponents, true);
            for (const UStaticMeshComponent* Mesh : MeshComponents)
            {
                if (IsValid(Mesh) && Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    DrawDebugBox(World, Mesh->Bounds.Origin, Mesh->Bounds.BoxExtent, Mesh->GetComponentQuat(), FColor(255, 180, 40), false, Duration, 0, 0.75f);
                    bDrewAny = true;
                }
            }
        }
    }

    if (!bDrewAny)
    {
        const FVector Location = GetComponentLocation();
        const FQuat Rotation = GetComponentQuat();
        if (FallbackShape == EMossCollisionShape::Box)
        {
            DrawDebugBox(World, Location, BoxHalfExtents.GetAbs() * GetComponentScale().GetAbs(), Rotation, Color, false, Duration, 0, 1.0f);
        }
        else if (FallbackShape == EMossCollisionShape::Sphere)
        {
            DrawDebugSphere(World, Location, Radius * FMath::Max3(GetComponentScale().GetAbs().X, GetComponentScale().GetAbs().Y, GetComponentScale().GetAbs().Z), 16, Color, false, Duration, 0, 1.0f);
        }
        else if (FallbackShape == EMossCollisionShape::Capsule)
        {
            const FVector Scale = GetComponentScale().GetAbs();
            const float RadialScale = static_cast<float>(FMath::Max(Scale.X, Scale.Y));
            DrawDebugCapsule(World, Location, CapsuleHalfHeight * static_cast<float>(Scale.Z), Radius * RadialScale, Rotation, Color, false, Duration, 0, 1.0f);
        }
    }
}
