#include "Audio/MossRayAudioStream3DComponent.h"
#include "Audio/AudioSystem.h"
bool UMossRayAudioStream3DComponent::TraceOcclusion(void* NativePhysicsSystem, const FVector& Listener, const FVector& Source,
    const FMossAudioRaycast3D& Raycast, FMossAudioRayTraceResult& OutResult, float MaxDistance)
{
    return FMossAudioSystem::Trace3D(NativePhysicsSystem, Listener, Source, MaxDistance, Raycast, OutResult);
}
