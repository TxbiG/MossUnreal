#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MossPhysicsCrateActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMossPhysicsBodyComponent;

/**
 * Minimal UnrealJolt-style Moss body example.
 * The Moss body component is the Actor root; visuals are children and the
 * query-only box provides the compound collision primitive imported by Moss.
 */
UCLASS()
class AMossPhysicsCrateActor : public AActor
{
    GENERATED_BODY()

public:
    AMossPhysicsCrateActor();

    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Example")
    bool Launch(const FVector& Direction, float ImpulseNewtonSeconds);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Moss|Physics|Example")
    TObjectPtr<UMossPhysicsBodyComponent> MossBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Moss|Physics|Example")
    TObjectPtr<UStaticMeshComponent> VisualMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Moss|Physics|Example")
    TObjectPtr<UBoxComponent> MossCollider;
};
