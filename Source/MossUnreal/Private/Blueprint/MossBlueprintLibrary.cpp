#include "Blueprint/MossBlueprintLibrary.h"

#include "Core/MossRuntime.h"
#include "Core/MossRuntimeSubsystem.h"
#include "Diagnostics/MossDiagnostics.h"
#include "Math/MossMath.h"
#include "Utilities/MossUtilities.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MossPhysicsWorldSubsystem.h"

bool UMossBlueprintLibrary::TryInitializeRuntime()
{
    return GEngine && GEngine->GetEngineSubsystem<UMossRuntimeSubsystem>()
        ? GEngine->GetEngineSubsystem<UMossRuntimeSubsystem>()->TryInitializeRuntime()
        : false;
}

bool UMossBlueprintLibrary::IsRuntimeAvailable()
{
    return FMossRuntime::Get().IsInitialized();
}

FString UMossBlueprintLibrary::GetRuntimeStatus()
{
    if (FMossRuntime::Get().IsInitialized())
    {
        const FMossBackendInfo Info = FMossRuntime::Get().GetBackendInfo();
        return FString::Printf(TEXT("Ready: %s %s (ABI %d)"), *Info.Name, *Info.Version, Info.AbiVersion);
    }
    if (GEngine)
    {
        if (const UMossRuntimeSubsystem* RuntimeSubsystem = GEngine->GetEngineSubsystem<UMossRuntimeSubsystem>())
        {
            const FString SubsystemStatus = RuntimeSubsystem->GetRuntimeStatus();
            if (!SubsystemStatus.IsEmpty())
            {
                return SubsystemStatus;
            }
        }
    }
    return FMossRuntime::Get().HasBackendFactory()
        ? TEXT("A native Moss backend is registered but has not initialized.")
        : TEXT("No native Moss backend is registered. Install or build the native backend provider.");
}

FString UMossBlueprintLibrary::GetBackendName()
{
    return FMossRuntime::Get().IsInitialized() ? FMossRuntime::Get().GetBackendInfo().Name : FString();
}

void UMossBlueprintLibrary::MossLog(const FString& Message)
{
    FMossDiagnostics::Log(Message);
}

void UMossBlueprintLibrary::MossWarning(const FString& Message)
{
    FMossDiagnostics::Warning(Message);
}

FString UMossBlueprintLibrary::MossMakeUniqueId()
{
    return FMossUtilities::MakeUniqueId();
}

double UMossBlueprintLibrary::MossRemap(const double Value, const double InputMinimum,
                                        const double InputMaximum, const double OutputMinimum,
                                        const double OutputMaximum, const bool bClamp)
{
    return FMossMath::Remap(Value, InputMinimum, InputMaximum, OutputMinimum, OutputMaximum, bClamp);
}

float UMossBlueprintLibrary::MossClampDeltaSeconds(const float DeltaSeconds, const float Maximum)
{
    return FMossUtilities::ClampDeltaSeconds(DeltaSeconds, Maximum);
}

bool UMossBlueprintLibrary::MossRaycast(const UObject* WorldContextObject,
                                        const FVector Start,
                                        const FVector End,
                                        FMossPhysicsRaycastHit& OutHit)
{
    OutHit = FMossPhysicsRaycastHit();
    if (!GEngine || !WorldContextObject)
    {
        return false;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    UMossPhysicsWorldSubsystem* MossWorld = World ? World->GetSubsystem<UMossPhysicsWorldSubsystem>() : nullptr;
    if (!MossWorld)
    {
        return false;
    }

    FString Error;
    const bool bSucceeded = MossWorld->Raycast(Start, End, OutHit, Error);
    if (!bSucceeded && !Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
    return bSucceeded && OutHit.bBlockingHit;
}
