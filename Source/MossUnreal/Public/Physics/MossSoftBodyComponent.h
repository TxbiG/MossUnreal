#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MossSoftBodyComponent.generated.h"

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossSoftBodyComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UMossSoftBodyComponent();
    UFUNCTION(BlueprintPure, Category="Moss|Physics|SoftBody") bool IsNativeSoftBodyReady() const { return NativeSoftBodyHandle != 0; }
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|SoftBody") bool RebuildNativeSoftBody(FString& OutError);
    UFUNCTION(BlueprintCallable, Category="Moss|Physics|SoftBody") void ResetNativeSoftBody();
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|SoftBody", meta=(ClampMin="1", ClampMax="64")) int32 GridResolution = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|SoftBody", meta=(ClampMin="0.1")) float GridSpacingCentimeters = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|SoftBody", meta=(ClampMin="1", ClampMax="32")) int32 SolverIterations = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Physics|SoftBody", meta=(ClampMin="0.0")) float Pressure = 0.0f;
private:
    uint64 NativeSoftBodyHandle = 0;
};
