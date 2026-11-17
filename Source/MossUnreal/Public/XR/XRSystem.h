#pragma once
#include "CoreMinimal.h"
#include "XR/XRDevice.h"

struct MOSSUNREAL_API FMossXRView
{
    FMossXRPose Pose;
    FMatrix View = FMatrix::Identity;
    FMatrix Projection = FMatrix::Identity;
    FVector4 FieldOfView = FVector4::Zero();
};

/** Moss XR is initialized against a Moss_Renderer pointer. An Unreal RHI renderer is not ABI-compatible by assumption. */
class MOSSUNREAL_API FMossXRSystem final
{
public:
    FMossXRSystem() = default;
    ~FMossXRSystem();
    FMossXRSystem(const FMossXRSystem&) = delete;
    FMossXRSystem& operator=(const FMossXRSystem&) = delete;
    FMossXRSystem(FMossXRSystem&& Other) noexcept;
    FMossXRSystem& operator=(FMossXRSystem&& Other) noexcept;

    bool Initialize(void* NativeMossRenderer);
    void Shutdown();
    bool IsInitialized() const { return NativeSession != nullptr; }
    bool BeginFrame();
    void EndFrame();
    void PollEvents();
    float GetDeltaSeconds() const;
    FMossXRCapabilities GetCapabilities() const;
    TArray<FMossXRView> GetViews() const;
    void* GetNativeSession() const { return NativeSession; }
private:
    void* NativeSession = nullptr;
    bool bOwnsGlobalInitialization = false;
};
