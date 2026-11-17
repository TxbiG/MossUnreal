#pragma once
#include "CoreMinimal.h"

/** Immediate-mode helpers over Moss/Moss_GUI.h. A renderer-owned Moss GUI context must already be active. */
class MOSSUNREAL_API FMossGUI final
{
public:
    static bool IsNativeAvailable();
    static bool IsContextManagementAvailable(); // Moss currently comments out its context create/set declarations.
    static bool BeginWindow(const FString& Name, bool* bOpen = nullptr, int32 NativeFlags = 0);
    static void EndWindow();
    static void Text(const FString& Value);
    static void RichText(const FString& Value);
    static void ProgressBar();
    static bool TextLink(const FString& Label);
    static bool RadioButton(const FString& Label, bool bActive);
    static bool ToggleSwitch(bool bActive);
    // The current Moss Slider* declarations take values by value and do not
    // return the edited value. Use DragFloat/DragInt for editable controls.
    static void SliderFloat(float Minimum, float Maximum, float Value);
    static void SliderInt(int32 Minimum, int32 Maximum, int32 Value);
    static bool DragFloat(const FString& Label, float& Value, float Speed = 1.0f, float Minimum = 0.0f, float Maximum = 0.0f);
    static bool DragInt(const FString& Label, int32& Value, float Speed = 1.0f, int32 Minimum = 0, int32 Maximum = 0);
};
