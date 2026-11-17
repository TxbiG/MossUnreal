#include "Audio/MossRayAudioStream2DComponent.h"
#include "Audio/AudioSystem.h"
bool UMossRayAudioStream2DComponent::TraceOcclusion(void* NativePhysicsSystem, const FVector2D& Listener, const FVector2D& Source,
    const FMossAudioRaycast2D& Raycast, FMossAudioRayTraceResult& OutResult, float MaxDistance)
{
    return FMossAudioSystem::Trace2D(NativePhysicsSystem, Listener, Source, MaxDistance, Raycast, OutResult);
}
