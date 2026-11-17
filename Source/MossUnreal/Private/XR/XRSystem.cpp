#include "XR/XRSystem.h"
#include "Core/MossNative.h"
#include "Misc/ScopeLock.h"

#if MOSSUNREAL_XR_API_AVAILABLE
namespace
{
    FCriticalSection GMossXRMutex;
    int32 GMossXRUsers = 0;
    Moss_Renderer* GMossXRRenderer = nullptr;

    bool AcquireMossXR(Moss_Renderer* Renderer)
    {
        FScopeLock Lock(&GMossXRMutex);
        if (!Renderer) return false;
        if (GMossXRUsers > 0 && GMossXRRenderer != Renderer) return false;
        if (GMossXRUsers == 0)
        {
            if (!Moss_XR_Initialize(Renderer)) return false;
            GMossXRRenderer = Renderer;
        }
        ++GMossXRUsers;
        return true;
    }

    void ReleaseMossXR()
    {
        FScopeLock Lock(&GMossXRMutex);
        if (GMossXRUsers <= 0) return;
        --GMossXRUsers;
        if (GMossXRUsers == 0)
        {
            Moss_XR_Shutdown();
            GMossXRRenderer = nullptr;
        }
    }
}
#endif

FMossXRSystem::~FMossXRSystem() { Shutdown(); }
FMossXRSystem::FMossXRSystem(FMossXRSystem&& Other) noexcept : NativeSession(Other.NativeSession), bOwnsGlobalInitialization(Other.bOwnsGlobalInitialization)
{ Other.NativeSession = nullptr; Other.bOwnsGlobalInitialization = false; }
FMossXRSystem& FMossXRSystem::operator=(FMossXRSystem&& Other) noexcept
{
    if (this != &Other) { Shutdown(); NativeSession = Other.NativeSession; bOwnsGlobalInitialization = Other.bOwnsGlobalInitialization; Other.NativeSession = nullptr; Other.bOwnsGlobalInitialization = false; }
    return *this;
}
bool FMossXRSystem::Initialize(void* NativeMossRenderer)
{
    Shutdown();
#if MOSSUNREAL_XR_API_AVAILABLE
    if (!NativeMossRenderer || !AcquireMossXR(static_cast<Moss_Renderer*>(NativeMossRenderer))) return false;
    bOwnsGlobalInitialization = true;
    NativeSession = Moss_XR_CreateSession();
    if (!NativeSession) Shutdown();
#endif
    return NativeSession != nullptr;
}
void FMossXRSystem::Shutdown()
{
#if MOSSUNREAL_XR_API_AVAILABLE
    if (NativeSession) Moss_XR_DestroySession(static_cast<MossXR_Session*>(NativeSession));
    if (bOwnsGlobalInitialization) ReleaseMossXR();
#endif
    NativeSession = nullptr; bOwnsGlobalInitialization = false;
}
bool FMossXRSystem::BeginFrame()
{
#if MOSSUNREAL_XR_API_AVAILABLE
    return NativeSession && Moss_XR_BeginFrame(static_cast<MossXR_Session*>(NativeSession));
#else
    return false;
#endif
}
void FMossXRSystem::EndFrame()
{
#if MOSSUNREAL_XR_API_AVAILABLE
    if (NativeSession) Moss_XR_EndFrame(static_cast<MossXR_Session*>(NativeSession));
#endif
}
void FMossXRSystem::PollEvents()
{
#if MOSSUNREAL_XR_API_AVAILABLE
    if (NativeSession) Moss_XR_HandleEvents(static_cast<MossXR_Session*>(NativeSession));
#endif
}
float FMossXRSystem::GetDeltaSeconds() const
{
#if MOSSUNREAL_XR_API_AVAILABLE
    return NativeSession ? Moss_XR_GetDeltaSeconds() : 0.0f;
#else
    return 0.0f;
#endif
}
FMossXRCapabilities FMossXRSystem::GetCapabilities() const
{
    FMossXRCapabilities Out;
#if MOSSUNREAL_XR_API_AVAILABLE
    if (NativeSession)
    {
        const MossXR_Capabilities* Native = Moss_XR_GetCapabilities();
        if (Native) { Out.ViewCount = Native->viewCount; Out.bHandTracking = Native->handTracking; Out.bEyeTracking = Native->eyeTracking;
            Out.bBodyTracking = Native->bodyTracking; Out.bFaceTracking = Native->faceTracking; Out.bPassthrough = Native->passthrough;
            Out.bAnchors = Native->anchors; Out.bDepthLayers = Native->depthLayers; }
    }
#endif
    return Out;
}
TArray<FMossXRView> FMossXRSystem::GetViews() const
{
    TArray<FMossXRView> Out;
#if MOSSUNREAL_XR_API_AVAILABLE
    if (!NativeSession) return Out;
    const uint32 Count = Moss_XR_GetViewCount(static_cast<MossXR_Session*>(NativeSession));
    Out.Reserve(static_cast<int32>(Count));
    for (uint32 I = 0; I < Count; ++I)
    {
        MossXR_View Native{};
        if (!Moss_XR_GetView(static_cast<MossXR_Session*>(NativeSession), I, &Native)) continue;
        FMossXRView View; View.Pose.Position = FVector(Native.pose.position.GetX(), Native.pose.position.GetY(), Native.pose.position.GetZ());
        View.Pose.Orientation = FQuat(Native.pose.orientation.GetX(), Native.pose.orientation.GetY(), Native.pose.orientation.GetZ(), Native.pose.orientation.GetW());
        View.FieldOfView = FVector4(Native.fov.left, Native.fov.right, Native.fov.up, Native.fov.down);
        // Matrix layout conversion is deliberately explicit; Moss matrix storage is not assumed to match FMatrix byte layout.
        for (int Row = 0; Row < 4; ++Row) for (int Col = 0; Col < 4; ++Col)
        { View.View.M[Row][Col] = Native.view.GetColumn4(static_cast<uint32>(Col))[static_cast<uint32>(Row)];
          View.Projection.M[Row][Col] = Native.projection.GetColumn4(static_cast<uint32>(Col))[static_cast<uint32>(Row)]; }
        Out.Add(View);
    }
#endif
    return Out;
}
