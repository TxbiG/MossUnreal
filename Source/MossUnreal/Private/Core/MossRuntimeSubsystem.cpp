#include "Core/MossRuntimeSubsystem.h"

#include "Core/MossRuntime.h"
#include "Diagnostics/MossDiagnostics.h"

void UMossRuntimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    TryInitializeRuntime();
}

bool UMossRuntimeSubsystem::TryInitializeRuntime()
{
    FMossRuntime& Runtime = FMossRuntime::Get();
    if (Runtime.IsInitialized())
    {
        bRuntimeReady = true;
        RuntimeStatus = TEXT("Native Moss runtime is initialized.");
        return true;
    }

    if (!Runtime.HasBackendFactory())
    {
        bRuntimeReady = false;
        bOwnsRuntimeLifecycle = false;
        RuntimeStatus = TEXT("No native Moss backend provider is registered yet.");
        return false;
    }

    FString Error;
    bRuntimeReady = Runtime.Initialize(FMossRuntimeConfig(), Error);
    RuntimeStatus = bRuntimeReady ? TEXT("Native Moss runtime initialized.") : Error;
    bOwnsRuntimeLifecycle = bRuntimeReady;

    if (bRuntimeReady)
    {
        FMossDiagnostics::Log(RuntimeStatus);
    }
    else if (!RuntimeStatus.IsEmpty())
    {
        FMossDiagnostics::Warning(RuntimeStatus);
    }
    return bRuntimeReady;
}

void UMossRuntimeSubsystem::Deinitialize()
{
    if (bOwnsRuntimeLifecycle)
    {
        FMossRuntime::Get().Shutdown();
    }
    bRuntimeReady = false;
    bOwnsRuntimeLifecycle = false;
    Super::Deinitialize();
}
