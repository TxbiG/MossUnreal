#pragma once
#include "Audio/MossAudioStream3DComponent.h"
#include "MossRayAudioStream3DComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossRayAudioStream3DComponent : public UMossAudioStream3DComponent
{
    GENERATED_BODY()
public:
    bool TraceOcclusion(void* NativePhysicsSystem, const FVector& Listener, const FVector& Source,
                        const FMossAudioRaycast3D& Raycast, FMossAudioRayTraceResult& OutResult, float MaxDistance = 100.0f);
};
