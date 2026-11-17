#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Physics/MossPhysicsTypes.h"
#include "MossPhysicsBodyComponent.generated.h"

class UMossPhysicsWorldSubsystem;

/** Optional Unreal component adapter for a Moss-owned physics body. */
UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossPhysicsBodyComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UMossPhysicsBodyComponent();

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    bool IsMossBodyReady() const { return NativeBody.IsValid(); }

    UFUNCTION(BlueprintCallable, Category="Moss|Physics")
    bool RecreateMossBody();

    UFUNCTION(BlueprintPure, Category="Moss|Physics")
    EMossBodyMode GetBodyMode() const { return BodyMode; }

    uint64 GetNativeBodyId() const { return NativeBody.Value; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    friend class UMossPhysicsWorldSubsystem;

    FMossNativeBodyDescription BuildNativeDescription() const;
    void SetNativeBodyHandle(FMossNativeBodyHandle Handle) { NativeBody = Handle; }
    void SynchronizeToNative(UMossPhysicsWorldSubsystem* WorldSubsystem);
    void SynchronizeFromNative(UMossPhysicsWorldSubsystem* WorldSubsystem);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true"))
    EMossBodyMode BodyMode = EMossBodyMode::Dynamic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true"))
    EMossCollisionShape CollisionShape = EMossCollisionShape::Box;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.001"))
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
    float AngularDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true"))
    bool bUseGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    FVector BoxHalfExtents = FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
    float HalfHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics", meta=(AllowPrivateAccess="true"))
    bool bSynchronizeTransform = true;

    FMossNativeBodyHandle NativeBody;
};
