#include "Systems/MossSystem.h"

bool FMossSystemRegistry::RegisterSystem(const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& System, FString& OutError)
{
    if (bInitialized)
    {
        OutError = TEXT("Systems cannot be registered after the registry has initialized. Shut it down first.");
        return false;
    }

    const FName Name = System->GetSystemName();
    if (Name.IsNone())
    {
        OutError = TEXT("A Moss system must provide a non-empty system name.");
        return false;
    }

    if (Systems.ContainsByPredicate([Name](const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& Existing)
        { return Existing->GetSystemName() == Name; }))
    {
        OutError = FString::Printf(TEXT("A Moss system named '%s' is already registered."), *Name.ToString());
        return false;
    }

    Systems.Add(System);
    SortSystems();
    OutError.Reset();
    return true;
}

bool FMossSystemRegistry::UnregisterSystem(const FName SystemName)
{
    if (bInitialized)
    {
        return false;
    }
    return Systems.RemoveAll([SystemName](const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& System)
        { return System->GetSystemName() == SystemName; }) > 0;
}

bool FMossSystemRegistry::Initialize(FString& OutError)
{
    if (bInitialized)
    {
        return true;
    }

    Context = FMossSystemContext();
    int32 InitializedCount = 0;
    for (const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& System : Systems)
    {
        FString SystemError;
        if (!System->Initialize(Context, SystemError))
        {
            for (int32 Index = InitializedCount - 1; Index >= 0; --Index)
            {
                Systems[Index]->Shutdown();
            }
            OutError = SystemError.IsEmpty()
                ? FString::Printf(TEXT("Moss system '%s' failed to initialize."), *System->GetSystemName().ToString())
                : SystemError;
            return false;
        }
        ++InitializedCount;
    }

    bInitialized = true;
    OutError.Reset();
    return true;
}

void FMossSystemRegistry::Tick(const double DeltaSeconds)
{
    if (!bInitialized)
    {
        return;
    }

    Context.DeltaSeconds = FMath::IsFinite(DeltaSeconds) ? FMath::Max(0.0, DeltaSeconds) : 0.0;
    Context.TotalSeconds += Context.DeltaSeconds;
    ++Context.FrameNumber;

    for (const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& System : Systems)
    {
        System->Tick(Context);
    }
}

void FMossSystemRegistry::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    for (int32 Index = Systems.Num() - 1; Index >= 0; --Index)
    {
        Systems[Index]->Shutdown();
    }
    bInitialized = false;
    Context = FMossSystemContext();
}

void FMossSystemRegistry::SortSystems()
{
    Systems.Sort([](const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& A,
                    const TSharedRef<IMossSystem, ESPMode::ThreadSafe>& B)
    {
        if (A->GetPriority() == B->GetPriority())
        {
            return A->GetSystemName().ToString() < B->GetSystemName().ToString();
        }
        return A->GetPriority() < B->GetPriority();
    });
}
