#include "Utilities/MossNativeTimer.h"
#include "Core/MossNative.h"

struct FMossNativeTimer::FImpl
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    explicit FImpl(float Duration, bool bRepeat) : NativeTimer(Duration, bRepeat) {}
    Timer NativeTimer;
#endif
};

FMossNativeTimer::FMossNativeTimer() = default;
FMossNativeTimer::~FMossNativeTimer() = default;
FMossNativeTimer::FMossNativeTimer(FMossNativeTimer&& Other) noexcept = default;
FMossNativeTimer& FMossNativeTimer::operator=(FMossNativeTimer&& Other) noexcept = default;

bool FMossNativeTimer::Start(float DurationSeconds, bool bRepeat, TFunction<void()> Callback)
{
    Impl.Reset();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (!FMath::IsFinite(DurationSeconds) || DurationSeconds < 0.0f)
        return false;
    Impl = MakeUnique<FImpl>(DurationSeconds, bRepeat);
    const std::shared_ptr<TFunction<void()>> SharedCallback =
        std::make_shared<TFunction<void()>>(MoveTemp(Callback));
    Impl->NativeTimer.setCallback([SharedCallback]()
    {
        if (SharedCallback && *SharedCallback)
            (*SharedCallback)();
    });
    return true;
#else
    return false;
#endif
}

void FMossNativeTimer::Update(float DeltaSeconds)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Impl && FMath::IsFinite(DeltaSeconds) && DeltaSeconds > 0.0f)
        Impl->NativeTimer.update(DeltaSeconds);
#endif
}
void FMossNativeTimer::Stop()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Impl) Impl->NativeTimer.stop();
#endif
}
void FMossNativeTimer::Reset()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Impl) Impl->NativeTimer.reset();
#endif
}
void FMossNativeTimer::Pause()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Impl) Impl->NativeTimer.pause();
#endif
}
void FMossNativeTimer::Resume()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (Impl) Impl->NativeTimer.resume();
#endif
}
bool FMossNativeTimer::IsRunning() const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Impl && Impl->NativeTimer.isRunning();
#else
    return false;
#endif
}
bool FMossNativeTimer::IsPaused() const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Impl && Impl->NativeTimer.isPaused();
#else
    return false;
#endif
}
float FMossNativeTimer::GetRemainingSeconds() const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Impl ? Impl->NativeTimer.getRemaining() : 0.0f;
#else
    return 0.0f;
#endif
}
bool FMossNativeTimer::IsNativeAvailable() const
{
    return MOSSUNREAL_NATIVE_API_AVAILABLE != 0;
}
