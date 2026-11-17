#include "XR/XRController.h"
#include "Core/MossNative.h"
FMossXRController::~FMossXRController() { Destroy(); }
FMossXRController::FMossXRController(FMossXRController&& Other) noexcept : NativeAction(Other.NativeAction) { Other.NativeAction = nullptr; }
FMossXRController& FMossXRController::operator=(FMossXRController&& Other) noexcept
{ if (this != &Other) { Destroy(); NativeAction = Other.NativeAction; Other.NativeAction = nullptr; } return *this; }
bool FMossXRController::CreatePoseAction(void* NativeActionSet, const FString& ActionName)
{
    Destroy();
#if MOSSUNREAL_XR_API_AVAILABLE
    if (!NativeActionSet || ActionName.IsEmpty()) return false;
    FTCHARToUTF8 UTF8(*ActionName);
    NativeAction = Moss_XR_CreateAction(static_cast<MossXR_ActionSet*>(NativeActionSet), UTF8.Get(), MossXR_ActionType::Pose);
#endif
    return NativeAction != nullptr;
}
void FMossXRController::Destroy()
{
#if MOSSUNREAL_XR_API_AVAILABLE
    if (NativeAction) Moss_XR_DestroyAction(static_cast<MossXR_Action*>(NativeAction));
#endif
    NativeAction = nullptr;
}
bool FMossXRController::GetPose(FMossXRPose& OutPose) const
{
#if MOSSUNREAL_XR_API_AVAILABLE
    if (!NativeAction) return false; MossXR_Pose Pose{};
    if (!Moss_XR_GetActionPose(static_cast<MossXR_Action*>(NativeAction), &Pose)) return false;
    OutPose.Position = FVector(Pose.position.GetX(), Pose.position.GetY(), Pose.position.GetZ());
    OutPose.Orientation = FQuat(Pose.orientation.GetX(), Pose.orientation.GetY(), Pose.orientation.GetZ(), Pose.orientation.GetW());
    return true;
#else
    return false;
#endif
}
