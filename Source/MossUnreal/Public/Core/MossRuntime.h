#pragma once

#include "CoreMinimal.h"
#include "Core/MossBackend.h"

/**
 * Explicit, engine-independent access point to Moss services.
 * A native backend must register a factory before Initialize() is called.
 */
class MOSSUNREAL_API FMossRuntime final
{
public:
    static FMossRuntime& Get();

    FMossRuntime(const FMossRuntime&) = delete;
    FMossRuntime& operator=(const FMossRuntime&) = delete;

    /** Register the bridge factory. Registration is rejected while initialized. */
    bool RegisterBackendFactory(FMossBackendFactory&& Factory, bool bReplaceExisting = false);

    /** Remove the registered factory when no backend is active or initializing. */
    bool UnregisterBackendFactory();

    /** Initialize the registered backend. Returns false when no provider is installed. */
    bool Initialize(const FMossRuntimeConfig& Config, FString& OutError);
    void Shutdown();

    bool IsInitialized() const;
    bool HasBackendFactory() const;
    FMossBackendInfo GetBackendInfo() const;
    TSharedPtr<IMossBackend, ESPMode::ThreadSafe> GetBackend() const;

private:
    FMossRuntime() = default;

    mutable FCriticalSection Mutex;
    FMossBackendFactory BackendFactory;
    TSharedPtr<IMossBackend, ESPMode::ThreadSafe> Backend;
    bool bInitializing = false;
};
