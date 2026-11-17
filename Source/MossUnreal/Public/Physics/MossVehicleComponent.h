#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MossVehicleComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossVehicleComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UMossVehicleComponent();
    UFUNCTION(BlueprintPure, Category="Moss|Physics|Vehicle") bool IsNativeVehicleReady() const { return NativeVehicleHandle != 0; }
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Vehicle") bool SetDriverInput(float Throttle, float Steering, float Brake, float HandBrake);
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|Vehicle") void ResetNativeVehicle();
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Vehicle", meta=(ClampMin="1", ClampMax="16")) int32 WheelCount = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Vehicle", meta=(ClampMin="0.0")) float MaxPitchRollDegrees = 65.0f;
private:
    uint64 NativeVehicleHandle = 0;
    float Throttle = 0.0f, Steering = 0.0f, Brake = 0.0f, HandBrake = 0.0f;
};
