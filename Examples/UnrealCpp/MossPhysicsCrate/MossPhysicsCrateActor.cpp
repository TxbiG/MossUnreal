#include "MossPhysicsCrateActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/MossPhysicsBodyComponent.h"

AMossPhysicsCrateActor::AMossPhysicsCrateActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MossBody = CreateDefaultSubobject<UMossPhysicsBodyComponent>(TEXT("MossPhysicsBody"));
    SetRootComponent(MossBody);

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(MossBody);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetGenerateOverlapEvents(false);

    MossCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("MossCollider"));
    MossCollider->SetupAttachment(MossBody);
    MossCollider->SetBoxExtent(FVector(50.0, 50.0, 50.0));
    // QueryOnly lets Unreal traces still see the collider, but Chaos does not
    // simulate it. Moss gathers this component to create the actual rigid body.
    MossCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MossCollider->SetGenerateOverlapEvents(false);
}

bool AMossPhysicsCrateActor::Launch(const FVector& Direction, const float ImpulseNewtonSeconds)
{
    if (!MossBody || ImpulseNewtonSeconds <= 0.0f)
    {
        return false;
    }

    return MossBody->AddImpulse(Direction.GetSafeNormal() * ImpulseNewtonSeconds);
}
