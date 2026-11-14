#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MossPhysicsWorldSubsystem.generated.h"

/**
 * Per-world owner for a future Moss simulation instance.
 *
 * This class currently provides Unreal lifecycle hooks only. It intentionally
 * does not call unverified Moss functions.
 */
UCLASS()
class MOSSUNREAL_API UMossPhysicsWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Whether the native Moss world has been successfully created. */
    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    bool IsMossWorldReady() const { return bMossWorldReady; }

private:
    bool bMossWorldReady = false;

    // Future native world handle belongs here after the public Moss API audit.
};
