#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MossAudioStream2DComponent.generated.h"

struct AudioStream2D;

UCLASS(ClassGroup=(Moss), meta=(BlueprintSpawnableComponent))
class MOSSUNREAL_API UMossAudioStream2DComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMossAudioStream2DComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category="Moss|Audio") bool CreateStream();
    UFUNCTION(BlueprintCallable, Category="Moss|Audio") void Play();
    UFUNCTION(BlueprintCallable, Category="Moss|Audio") void Stop();
    UFUNCTION(BlueprintCallable, Category="Moss|Audio") void SetVolume(float Value);
    UFUNCTION(BlueprintCallable, Category="Moss|Audio") void SetPitch(float Value);
    UFUNCTION(BlueprintCallable, Category="Moss|Audio") void SetLooping(bool bLoop);
    UFUNCTION(BlueprintPure, Category="Moss|Audio") bool IsStreamValid() const { return NativeStream != nullptr; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Audio") bool bAutoPlay = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Audio", meta=(ClampMin="0.0")) float Volume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Audio", meta=(ClampMin="0.01")) float Pitch = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Moss|Audio") bool bLoop = false;
    AudioStream2D* NativeStream = nullptr;
};
