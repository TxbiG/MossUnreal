#include "Audio/AudioSystem.h"
#include "Core/MossNative.h"
#include "Misc/ScopeLock.h"

#if MOSSUNREAL_NATIVE_API_AVAILABLE
namespace
{
    FCriticalSection GMossAudioMutex;
    int32 GMossAudioUsers = 0;

    bool AcquireMossAudio()
    {
        FScopeLock Lock(&GMossAudioMutex);
        if (GMossAudioUsers == 0 && Moss_Init_Audio() != 0)
            return false;
        ++GMossAudioUsers;
        return true;
    }

    void ReleaseMossAudio()
    {
        FScopeLock Lock(&GMossAudioMutex);
        if (GMossAudioUsers <= 0)
            return;
        --GMossAudioUsers;
        if (GMossAudioUsers == 0)
            Moss_Terminate_Audio();
    }

    struct FAudioRay3DContext { const FMossAudioRaycast3D* Callback; void* UserData; };
    struct FAudioRay2DContext { const FMossAudioRaycast2D* Callback; void* UserData; };

    bool MossRaycast3DThunk(PhysicsSystem* Physics, const Vec3* Origin, const Vec3* Direction, float Distance, Moss_AudioRayHit3D* OutHit, void* ContextPtr)
    {
        FAudioRay3DContext* Context = static_cast<FAudioRay3DContext*>(ContextPtr);
        if (!Context || !Context->Callback || !Context->Callback->IsBound() || !Origin || !Direction || !OutHit) return false;
        FMossAudioRayHit Hit;
        if (!(*Context->Callback)(FVector(Origin->GetX(), Origin->GetY(), Origin->GetZ()), FVector(Direction->GetX(), Direction->GetY(), Direction->GetZ()), Distance, Hit)) return false;
        OutHit->hit = Hit.bHit; OutHit->fraction = Hit.Fraction;
        OutHit->position = Vec3(Hit.Position.X, Hit.Position.Y, Hit.Position.Z);
        OutHit->normal = Vec3(Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z);
        OutHit->absorption = Hit.Absorption; OutHit->transmission = Hit.Transmission; OutHit->material_id = Hit.MaterialId;
        return true;
    }

    bool MossRaycast2DThunk(PhysicsSystem* Physics, const Vec2* Origin, const Vec2* Direction, float Distance, Moss_AudioRayHit2D* OutHit, void* ContextPtr)
    {
        FAudioRay2DContext* Context = static_cast<FAudioRay2DContext*>(ContextPtr);
        if (!Context || !Context->Callback || !Context->Callback->IsBound() || !Origin || !Direction || !OutHit) return false;
        FMossAudioRayHit Hit;
        if (!(*Context->Callback)(FVector2D(Origin->GetX(), Origin->GetY()), FVector2D(Direction->GetX(), Direction->GetY()), Distance, Hit)) return false;
        OutHit->hit = Hit.bHit; OutHit->fraction = Hit.Fraction;
        OutHit->position = Vec2(Hit.Position.X, Hit.Position.Y);
        OutHit->normal = Vec2(Hit.Normal.X, Hit.Normal.Y);
        OutHit->absorption = Hit.Absorption; OutHit->transmission = Hit.Transmission; OutHit->material_id = Hit.MaterialId;
        return true;
    }

    void CopyTraceResult(const Moss_AudioRayTraceResult& Native, FMossAudioRayTraceResult& Out)
    {
        Out.bAudible = Native.audible; Out.bOccluded = Native.occluded; Out.Distance = Native.distance;
        Out.Attenuation = Native.attenuation; Out.Occlusion = Native.occlusion; Out.TransmissionGain = Native.transmission_gain;
        Out.LowPass = Native.lowpass; Out.ReflectionGain = Native.reflection_gain; Out.DelaySeconds = Native.delay_seconds;
    }
}
#endif

bool FMossAudioSystem::Initialize()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return AcquireMossAudio();
#else
    return false;
#endif
}
void FMossAudioSystem::Shutdown()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    ReleaseMossAudio();
#endif
}
void FMossAudioSystem::Update(float DeltaSeconds)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (FMath::IsFinite(DeltaSeconds) && DeltaSeconds >= 0.0f) Moss_AudioUpdate(DeltaSeconds);
#endif
}
bool FMossAudioSystem::IsAvailable() { return MOSSUNREAL_NATIVE_API_AVAILABLE != 0; }
Moss_AudioSource* FMossAudioSystem::LoadWav(const FString& FilePath)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*FilePath); return Moss_AudioLoadWavFile(UTF8.Get());
#else
    return nullptr;
#endif
}
Moss_AudioSource* FMossAudioSystem::LoadOgg(const FString& FilePath, int32 NativeLoadType)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*FilePath); return Moss_AudioLoadOgg(UTF8.Get(), static_cast<AudioLoadType>(NativeLoadType));
#else
    return nullptr;
#endif
}
Moss_AudioSource* FMossAudioSystem::LoadMp3(const FString& FilePath)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*FilePath); return Moss_AudioLoadMP3(UTF8.Get());
#else
    return nullptr;
#endif
}
void FMossAudioSystem::DestroySource(Moss_AudioSource* Source)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Source) Moss_AudioSourceDestroy(Source);
#endif
}
bool FMossAudioSystem::Trace3D(void* NativePhysicsSystem, const FVector& Listener, const FVector& Source, float MaxDistance,
                               const FMossAudioRaycast3D& Raycast, FMossAudioRayTraceResult& OutResult, void* UserData)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativePhysicsSystem || !Raycast) return false;
    FAudioRay3DContext Context{&Raycast, UserData};
    Moss_AudioRayTrace3DDesc Desc;
    Desc.physics = static_cast<PhysicsSystem*>(NativePhysicsSystem); Desc.raycast = &MossRaycast3DThunk; Desc.user_data = &Context;
    Desc.listener_position = Vec3(Listener.X, Listener.Y, Listener.Z); Desc.source_position = Vec3(Source.X, Source.Y, Source.Z);
    Desc.max_distance = FMath::Max(0.0f, MaxDistance);
    Moss_AudioRayTraceResult NativeResult{};
    if (!Moss_AudioRayTrace3D(&Desc, &NativeResult)) return false;
    CopyTraceResult(NativeResult, OutResult); return true;
#else
    return false;
#endif
}
bool FMossAudioSystem::Trace2D(void* NativePhysicsSystem, const FVector2D& Listener, const FVector2D& Source, float MaxDistance,
                               const FMossAudioRaycast2D& Raycast, FMossAudioRayTraceResult& OutResult, void* UserData)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativePhysicsSystem || !Raycast) return false;
    FAudioRay2DContext Context{&Raycast, UserData};
    Moss_AudioRayTrace2DDesc Desc;
    Desc.physics = static_cast<PhysicsSystem*>(NativePhysicsSystem); Desc.raycast = &MossRaycast2DThunk; Desc.user_data = &Context;
    Desc.listener_position = Vec2(Listener.X, Listener.Y); Desc.source_position = Vec2(Source.X, Source.Y);
    Desc.max_distance = FMath::Max(0.0f, MaxDistance);
    Moss_AudioRayTraceResult NativeResult{};
    if (!Moss_AudioRayTrace2D(&Desc, &NativeResult)) return false;
    CopyTraceResult(NativeResult, OutResult); return true;
#else
    return false;
#endif
}
