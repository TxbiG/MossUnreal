#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MossPhysicsBodyComponent.generated.h"

/**
 * Unreal-facing entry point for a Moss-owned physics body.
 *
 * This is not a replacement for UPrimitiveComponent or Chaos yet. The native
 * body handle and synchronisation are intentionally deferred until the Moss
 * public API and ownership model have been verified.
 */
UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossPhysicsBodyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMossPhysicsBodyComponent();

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    bool IsMossBodyReady() const { return bMossBodyReady; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(VisibleAnywhere, Category="Moss|Physics")
    bool bMossBodyReady = false;

    // Future: explicit body configuration, shape source and native handle.
};
