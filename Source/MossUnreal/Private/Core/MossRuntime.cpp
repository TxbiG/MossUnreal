#include "Core/MossRuntime.h"

#include "Misc/ScopeLock.h"

FMossRuntime& FMossRuntime::Get()
{
    static FMossRuntime Runtime;
    return Runtime;
}

bool FMossRuntime::RegisterBackendFactory(FMossBackendFactory&& Factory, const bool bReplaceExisting)
{
    if (!Factory.IsSet())
    {
        return false;
    }

    FScopeLock Lock(&Mutex);
    if (Backend.IsValid() || bInitializing || (BackendFactory.IsSet() && !bReplaceExisting))
    {
        return false;
    }

    BackendFactory = MoveTemp(Factory);
    return true;
}

bool FMossRuntime::UnregisterBackendFactory()
{
    FScopeLock Lock(&Mutex);
    if (Backend.IsValid() || bInitializing || !BackendFactory.IsSet())
    {
        return false;
    }
    BackendFactory = FMossBackendFactory();
    return true;
}

bool FMossRuntime::Initialize(const FMossRuntimeConfig& Config, FString& OutError)
{
    FMossBackendFactory Factory;
    TSharedPtr<IMossBackend, ESPMode::ThreadSafe> ExistingBackend;
    {
        FScopeLock Lock(&Mutex);
        if (bInitializing)
        {
            OutError = TEXT("Moss runtime initialization is already in progress.");
            return false;
        }

        ExistingBackend = Backend;
        if (!ExistingBackend.IsValid())
        {
            if (!BackendFactory.IsSet())
            {
                OutError = TEXT("No native Moss backend is registered. Build and load a backend provider against the public Moss headers, then register its factory before runtime initialization.");
                return false;
            }
            bInitializing = true;
            Factory = BackendFactory;
        }
    }

    // Never call provider code while holding the runtime mutex.
    if (ExistingBackend.IsValid())
    {
        if (ExistingBackend->IsInitialized())
        {
            OutError.Reset();
            return true;
        }

        ExistingBackend->Shutdown();
        {
            FScopeLock Lock(&Mutex);
            if (Backend == ExistingBackend)
            {
                Backend.Reset();
            }
            if (bInitializing)
            {
                OutError = TEXT("Moss runtime initialization is already in progress.");
                return false;
            }
            if (!BackendFactory.IsSet())
            {
                OutError = TEXT("The previous Moss backend was not initialized and no provider factory is registered.");
                return false;
            }
            bInitializing = true;
            Factory = BackendFactory;
        }
    }

    TSharedPtr<IMossBackend, ESPMode::ThreadSafe> NewBackend = Factory();
    if (!NewBackend.IsValid())
    {
        FScopeLock Lock(&Mutex);
        bInitializing = false;
        OutError = TEXT("The registered Moss backend factory returned an invalid backend.");
        return false;
    }

    FString BackendError;
    bool bSuccess = NewBackend->Initialize(Config, BackendError) && NewBackend->IsInitialized();
    if (bSuccess)
    {
        const FMossBackendInfo Info = NewBackend->GetInfo();
        if (Info.AbiVersion != Config.ExpectedBackendAbi)
        {
            bSuccess = false;
            BackendError = FString::Printf(
                TEXT("Moss backend ABI mismatch. Expected provider ABI %d, received %d."),
                Config.ExpectedBackendAbi, Info.AbiVersion);
        }
    }

    {
        FScopeLock Lock(&Mutex);
        bInitializing = false;
        if (bSuccess)
        {
            Backend = MoveTemp(NewBackend);
        }
    }

    if (!bSuccess)
    {
        NewBackend->Shutdown();
        OutError = BackendError.IsEmpty()
            ? TEXT("The native Moss backend failed to initialize or did not report an initialized state.")
            : BackendError;
    }
    else
    {
        OutError.Reset();
    }
    return bSuccess;
}

void FMossRuntime::Shutdown()
{
    TSharedPtr<IMossBackend, ESPMode::ThreadSafe> BackendToStop;
    {
        FScopeLock Lock(&Mutex);
        BackendToStop = Backend;
        Backend.Reset();
    }
    if (BackendToStop.IsValid())
    {
        BackendToStop->Shutdown();
    }
}

bool FMossRuntime::IsInitialized() const
{
    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> CurrentBackend = GetBackend();
    return CurrentBackend.IsValid() && CurrentBackend->IsInitialized();
}

bool FMossRuntime::HasBackendFactory() const
{
    FScopeLock Lock(&Mutex);
    return BackendFactory.IsSet();
}

FMossBackendInfo FMossRuntime::GetBackendInfo() const
{
    const TSharedPtr<IMossBackend, ESPMode::ThreadSafe> CurrentBackend = GetBackend();
    return CurrentBackend.IsValid() ? CurrentBackend->GetInfo() : FMossBackendInfo();
}

TSharedPtr<IMossBackend, ESPMode::ThreadSafe> FMossRuntime::GetBackend() const
{
    FScopeLock Lock(&Mutex);
    return Backend;
}
