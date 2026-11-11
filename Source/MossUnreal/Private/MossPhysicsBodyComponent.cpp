#include "MossPhysicsBodyComponent.h"
#include "MossPhysicsWorldSubsystem.h"
#include "Engine/World.h"

UMossPhysicsBodyComponent::UMossPhysicsBodyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMossPhysicsBodyComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (const UMossPhysicsWorldSubsystem* MossWorld =
            World->GetSubsystem<UMossPhysicsWorldSubsystem>())
        {
            // Native body creation will be added after the adapter is implemented.
            bMossBodyReady = false;
        }
    }
}

void UMossPhysicsBodyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // TODO: destroy the native body before the owning actor/world disappears.
    bMossBodyReady = false;

    Super::EndPlay(EndPlayReason);
}
