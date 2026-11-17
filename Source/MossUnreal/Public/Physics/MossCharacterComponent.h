#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MossCharacterComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossCharacterComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UMossCharacterComponent();
    UFUNCTION(BlueprintPure, Category="Moss|Physics|Character") bool IsNativeCharacterReady() const { return NativeCharacterHandle != 0; }
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Character") bool SetDesiredVelocity(const FVector& VelocityMetersPerSecond);
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Character") void ResetNativeCharacter();
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Character", meta=(ClampMin="0.01")) float CapsuleRadius = 34.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Character", meta=(ClampMin="0.01")) float CapsuleHalfHeight = 88.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Character", meta=(ClampMin="0.0", ClampMax="89.0")) float MaxSlopeAngleDegrees = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Character", meta=(ClampMin="0.0")) float GravityFactor = 1.0f;
private:
    uint64 NativeCharacterHandle = 0;
};
