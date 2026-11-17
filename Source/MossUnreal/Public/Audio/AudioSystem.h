#pragma once

#include "CoreMinimal.h"

struct Moss_AudioSource;

struct MOSSUNREAL_API FMossAudioRayHit
{
    bool bHit = false;
    FVector Position = FVector::ZeroVector;
    FVector Normal = FVector::UpVector;
    float Fraction = 1.0f;
    float Absorption = 0.5f;
    float Transmission = 0.25f;
    uint32 MaterialId = 0;
};

struct MOSSUNREAL_API FMossAudioRayTraceResult
{
    bool bAudible = false;
    bool bOccluded = false;
    float Distance = 0.0f;
    float Attenuation = 1.0f;
    float Occlusion = 0.0f;
    float TransmissionGain = 1.0f;
    float LowPass = 1.0f;
    float ReflectionGain = 0.0f;
    float DelaySeconds = 0.0f;
};

using FMossAudioRaycast3D = TFunction<bool(const FVector&, const FVector&, float, FMossAudioRayHit&)>;
using FMossAudioRaycast2D = TFunction<bool(const FVector2D&, const FVector2D&, float, FMossAudioRayHit&)>;

/** Direct wrappers over Moss/Moss_Audio.h. */
class MOSSUNREAL_API FMossAudioSystem final
{
public:
    static bool Initialize();
    static void Shutdown();
    static void Update(float DeltaSeconds);
    static bool IsAvailable();
    static Moss_AudioSource* LoadWav(const FString& FilePath);
    static Moss_AudioSource* LoadOgg(const FString& FilePath, int32 NativeLoadType);
    static Moss_AudioSource* LoadMp3(const FString& FilePath);
    static void DestroySource(Moss_AudioSource* Source);
    static bool Trace3D(void* NativePhysicsSystem, const FVector& Listener, const FVector& Source,
                        float MaxDistance, const FMossAudioRaycast3D& Raycast, FMossAudioRayTraceResult& OutResult,
                        void* UserData = nullptr);
    static bool Trace2D(void* NativePhysicsSystem, const FVector2D& Listener, const FVector2D& Source,
                        float MaxDistance, const FMossAudioRaycast2D& Raycast, FMossAudioRayTraceResult& OutResult,
                        void* UserData = nullptr);
};
