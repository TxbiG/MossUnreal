#include "Audio/MossAudioStream2DComponent.h"
#include "Audio/AudioSystem.h"
#include "Core/MossNative.h"

UMossAudioStream2DComponent::UMossAudioStream2DComponent() { PrimaryComponentTick.bCanEverTick = false; }
void UMossAudioStream2DComponent::BeginPlay() { Super::BeginPlay(); if (bAutoPlay) { CreateStream(); Play(); } }
void UMossAudioStream2DComponent::EndPlay(const EEndPlayReason::Type Reason)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DRemove(NativeStream);
#endif
    NativeStream = nullptr; Super::EndPlay(Reason);
}
bool UMossAudioStream2DComponent::CreateStream()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativeStream) NativeStream = Moss_AudioStream2DCreate();
    if (NativeStream) { SetVolume(Volume); SetPitch(Pitch); SetLooping(bLoop); }
#endif
    return NativeStream != nullptr;
}
void UMossAudioStream2DComponent::Play()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DPlay(NativeStream);
#endif
}
void UMossAudioStream2DComponent::Stop()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DStop(NativeStream);
#endif
}
void UMossAudioStream2DComponent::SetVolume(float Value)
{
    Volume = FMath::Max(0.0f, Value);
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DSetVolume(NativeStream, Volume);
#endif
}
void UMossAudioStream2DComponent::SetPitch(float Value)
{
    Pitch = FMath::Max(0.01f, Value);
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DSetPitch(NativeStream, Pitch);
#endif
}
void UMossAudioStream2DComponent::SetLooping(bool bLoopValue)
{
    bLoop = bLoopValue;
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream2DSetLoop(NativeStream, bLoop);
#endif
}
