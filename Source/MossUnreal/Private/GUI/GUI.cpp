#include "GUI/GUI.h"
#include "Core/MossNative.h"
bool FMossGUI::IsNativeAvailable() { return MOSSUNREAL_GUI_API_AVAILABLE != 0; }
bool FMossGUI::IsContextManagementAvailable() { return false; }
bool FMossGUI::BeginWindow(const FString& Name, bool* bOpen, int32 Flags)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Name); return ::BeginWindow(UTF8.Get(), bOpen, Flags);
#else
    return false;
#endif
}
void FMossGUI::EndWindow()
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    ::EndWindow();
#endif
}
void FMossGUI::Text(const FString& Value)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Value); ::Text("%s", UTF8.Get());
#endif
}
void FMossGUI::RichText(const FString& Value)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Value); ::RichText(UTF8.Get());
#endif
}
void FMossGUI::ProgressBar()
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    ::ProgressBar();
#endif
}
bool FMossGUI::TextLink(const FString& Label)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Label); return ::TextLink(UTF8.Get());
#else
    return false;
#endif
}
bool FMossGUI::RadioButton(const FString& Label, bool bActive)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Label); return ::RadioButton(UTF8.Get(), bActive);
#else
    return false;
#endif
}
bool FMossGUI::ToggleSwitch(bool bActive)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    return ::ToggleSwitch(bActive);
#else
    return bActive;
#endif
}
void FMossGUI::SliderFloat(float Minimum, float Maximum, float Value)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    ::SliderFloat(Minimum, Maximum, Value);
#endif
}
void FMossGUI::SliderInt(int32 Minimum, int32 Maximum, int32 Value)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    ::SliderInt(Minimum, Maximum, Value);
#endif
}


bool FMossGUI::DragFloat(const FString& Label, float& Value, float Speed, float Minimum, float Maximum)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Label);
    return ::DragFloat(UTF8.Get(), &Value, Speed, Minimum, Maximum);
#else
    return false;
#endif
}

bool FMossGUI::DragInt(const FString& Label, int32& Value, float Speed, int32 Minimum, int32 Maximum)
{
#if MOSSUNREAL_GUI_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Label);
    return ::DragInt(UTF8.Get(), &Value, Speed, Minimum, Maximum);
#else
    return false;
#endif
}
