#include "MossPhysicsWorldSubsystem.h"

void UMossPhysicsWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // TODO: create the native Moss world through a verified adapter.
    // Do not set bMossWorldReady until creation and ABI validation succeed.
    bMossWorldReady = false;
}

void UMossPhysicsWorldSubsystem::Deinitialize()
{
    // TODO: stop stepping, unregister callbacks, destroy native objects,
    // and release the native world before the Unreal world is torn down.
    bMossWorldReady = false;

    Super::Deinitialize();
}
