#include "Audio/MossAudioStream3DComponent.h"
#include "Core/MossNative.h"

UMossAudioStream3DComponent::UMossAudioStream3DComponent() { PrimaryComponentTick.bCanEverTick = false; }
void UMossAudioStream3DComponent::BeginPlay() { Super::BeginPlay(); if (bAutoPlay) { CreateStream(); Play(); } }
void UMossAudioStream3DComponent::EndPlay(const EEndPlayReason::Type Reason)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DRemove(NativeStream);
#endif
    NativeStream = nullptr; Super::EndPlay(Reason);
}
bool UMossAudioStream3DComponent::CreateStream()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!NativeStream) NativeStream = Moss_AudioStream3DCreate();
    if (NativeStream) { SetVolume(Volume); SetPitch(Pitch); SetLooping(bLoop); }
#endif
    return NativeStream != nullptr;
}
void UMossAudioStream3DComponent::Play()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DPlay(NativeStream);
#endif
}
void UMossAudioStream3DComponent::Stop()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DStop(NativeStream);
#endif
}
void UMossAudioStream3DComponent::SetVolume(float Value)
{
    Volume = FMath::Max(0.0f, Value);
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DSetVolume(NativeStream, Volume);
#endif
}
void UMossAudioStream3DComponent::SetPitch(float Value)
{
    Pitch = FMath::Max(0.01f, Value);
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DSetPitch(NativeStream, Pitch);
#endif
}
void UMossAudioStream3DComponent::SetLooping(bool bLoopValue)
{
    bLoop = bLoopValue;
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeStream) Moss_AudioStream3DSetLoop(NativeStream, bLoop);
#endif
}
