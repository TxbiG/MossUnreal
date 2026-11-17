#pragma once
#include "CoreMinimal.h"
#include "XR/XRDevice.h"

/** Small owner for a Moss pose action; action-set attachment/sync remains an explicit session operation. */
class MOSSUNREAL_API FMossXRController final
{
public:
    FMossXRController() = default;
    ~FMossXRController();
    FMossXRController(const FMossXRController&) = delete;
    FMossXRController& operator=(const FMossXRController&) = delete;
    FMossXRController(FMossXRController&& Other) noexcept;
    FMossXRController& operator=(FMossXRController&& Other) noexcept;
    bool CreatePoseAction(void* NativeActionSet, const FString& ActionName);
    void Destroy();
    bool GetPose(FMossXRPose& OutPose) const;
    bool IsValid() const { return NativeAction != nullptr; }
private:
    void* NativeAction = nullptr;
};
