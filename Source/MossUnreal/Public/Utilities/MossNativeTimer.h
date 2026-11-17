#pragma once

#include "CoreMinimal.h"

/** Code-first RAII wrapper around Moss's native Timer class. */
class MOSSUNREAL_API FMossNativeTimer final
{
public:
    FMossNativeTimer();
    ~FMossNativeTimer();
    FMossNativeTimer(const FMossNativeTimer&) = delete;
    FMossNativeTimer& operator=(const FMossNativeTimer&) = delete;
    FMossNativeTimer(FMossNativeTimer&& Other) noexcept;
    FMossNativeTimer& operator=(FMossNativeTimer&& Other) noexcept;

    bool Start(float DurationSeconds, bool bRepeat, TFunction<void()> Callback);
    void Update(float DeltaSeconds);
    void Stop();
    void Reset();
    void Pause();
    void Resume();
    bool IsRunning() const;
    bool IsPaused() const;
    float GetRemainingSeconds() const;
    bool IsNativeAvailable() const;

private:
    struct FImpl;
    TUniquePtr<FImpl> Impl;
};
