#pragma once
#include "Audio/MossAudioStream2DComponent.h"
#include "MossRayAudioStream2DComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossRayAudioStream2DComponent : public UMossAudioStream2DComponent
{
    GENERATED_BODY()
public:
    bool TraceOcclusion(void* NativePhysicsSystem, const FVector2D& Listener, const FVector2D& Source,
                        const FMossAudioRaycast2D& Raycast, FMossAudioRayTraceResult& OutResult, float MaxDistance = 100.0f);
};
