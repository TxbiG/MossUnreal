#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "MossRuntimeSubsystem.generated.h"

/** Unreal-managed lifecycle owner for the process-wide Moss runtime. */
UCLASS()
class MOSSUNREAL_API UMossRuntimeSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Retry after an optional provider module has registered its backend factory. */
    UFUNCTION(BlueprintCallable, Category="Moss|Runtime")
    bool TryInitializeRuntime();

    UFUNCTION(BlueprintPure, Category="Moss|Runtime")
    bool IsRuntimeReady() const { return bRuntimeReady; }

    UFUNCTION(BlueprintPure, Category="Moss|Runtime")
    FString GetRuntimeStatus() const { return RuntimeStatus; }

private:
    bool bRuntimeReady = false;
    bool bOwnsRuntimeLifecycle = false;
    FString RuntimeStatus;
};
