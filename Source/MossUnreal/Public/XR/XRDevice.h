#pragma once
#include "CoreMinimal.h"

struct MOSSUNREAL_API FMossXRPose
{
    FVector Position = FVector::ZeroVector;
    FQuat Orientation = FQuat::Identity;
};
struct MOSSUNREAL_API FMossXRCapabilities
{
    uint32 ViewCount = 0;
    bool bHandTracking = false;
    bool bEyeTracking = false;
    bool bBodyTracking = false;
    bool bFaceTracking = false;
    bool bPassthrough = false;
    bool bAnchors = false;
    bool bDepthLayers = false;
};
