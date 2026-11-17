#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MossWheelComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossWheelComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UMossWheelComponent();
    UFUNCTION(BlueprintPure, Category="Moss|Physics|Vehicle") float GetRadiusMeters() const { return RadiusCentimeters * 0.01f; }
    UFUNCTION(BlueprintPure, Category="Moss|Physics|Vehicle") float GetWidthMeters() const { return WidthCentimeters * 0.01f; }
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Vehicle", meta=(ClampMin="0.1")) float RadiusCentimeters = 35.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Vehicle", meta=(ClampMin="0.1")) float WidthCentimeters = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|Vehicle", meta=(ClampMin="0.0")) float MaxBrakeTorque = 1500.0f;
};
