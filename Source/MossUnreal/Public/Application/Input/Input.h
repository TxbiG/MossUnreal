#pragma once

#include "CoreMinimal.h"

/** Input queries against Moss's platform input state, not Unreal's Enhanced Input stack. */
class MOSSUNREAL_API FMossInput final
{
public:
    static bool IsNativeAvailable();
    static bool IsKeyDown(const FKey& Key);
    static bool IsKeyReleased(const FKey& Key);
    static bool WasKeyJustPressed(const FKey& Key);
    static bool WasKeyJustReleased(const FKey& Key);
    static bool IsMouseButtonDown(const FKey& Button);
    static bool WasMouseButtonJustPressed(const FKey& Button);
    static bool WasMouseButtonJustReleased(const FKey& Button);
    static FVector2D GetMousePosition();
    static void SetMousePosition(const FVector2D& Position);
    static void SetMouseVisible(bool bVisible);
};
