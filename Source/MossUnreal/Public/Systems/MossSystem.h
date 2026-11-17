#pragma once

#include "CoreMinimal.h"

/** Update context intentionally contains no UObject dependency. */
struct MOSSUNREAL_API FMossSystemContext
{
    double DeltaSeconds = 0.0;
    double TotalSeconds = 0.0;
    uint64 FrameNumber = 0;
};

/**
 * Native C++ system contract. Systems are ordinary C++ objects and do not need
 * to inherit from AActor, UActorComponent or UObject.
 */
class MOSSUNREAL_API IMossSystem
{
public:
    virtual ~IMossSystem() = default;
    virtual FName GetSystemName() const = 0;
    virtual int32 GetPriority() const { return 0; }
    virtual bool Initialize(const FMossSystemContext& Context, FString& OutError) = 0;
    virtual void Tick(const FMossSystemContext& Context) = 0;
    virtual void Shutdown() = 0;
};

/** Game-thread-owned, deterministic system collection. */
class MOSSUNREAL_API FMossSystemRegistry final
{
public:
    bool RegisterSystem(const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& System, FString& OutError);
    bool UnregisterSystem(FName SystemName);
    bool Initialize(FString& OutError);
    void Tick(double DeltaSeconds);
    void Shutdown();
    bool IsInitialized() const { return bInitialized; }
    int32 Num() const { return Systems.Num(); }

private:
    void SortSystems();
    TArray<TSharedRef<IMossSystem, ESPMode::ThreadSafe>> Systems;
    FMossSystemContext Context;
    bool bInitialized = false;
};
