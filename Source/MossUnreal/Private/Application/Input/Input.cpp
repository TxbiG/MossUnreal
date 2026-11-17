#include "Application/Input/Input.h"
#include "Core/MossNative.h"

#if MOSSUNREAL_NATIVE_API_AVAILABLE
static bool TryMossKeyboardKey(const FKey& Key, Keyboard& OutKey)
{
    const FString Name = Key.GetFName().ToString().ToUpper();
    if (Name.Len() == 1)
    {
        const TCHAR C = Name[0];
        if (C >= TEXT('A') && C <= TEXT('Z'))
        {
            OutKey = static_cast<Keyboard>(static_cast<int32>(Keyboard::KEY_A) + (C - TEXT('A')));
            return true;
        }
        if (C >= TEXT('0') && C <= TEXT('9'))
        {
            OutKey = static_cast<Keyboard>(static_cast<int32>(Keyboard::KEY_0) + (C - TEXT('0')));
            return true;
        }
    }
    static const TMap<FString, Keyboard> Special = {
        {TEXT("SPACEBAR"), Keyboard::KEY_SPACE}, {TEXT("ENTER"), Keyboard::KEY_ENTER},
        {TEXT("ESCAPE"), Keyboard::KEY_ESCAPE}, {TEXT("TAB"), Keyboard::KEY_TAB},
        {TEXT("BACKSPACE"), Keyboard::KEY_BACKSPACE}, {TEXT("DELETE"), Keyboard::KEY_DELETE},
        {TEXT("INSERT"), Keyboard::KEY_INSERT}, {TEXT("HOME"), Keyboard::KEY_HOME},
        {TEXT("END"), Keyboard::KEY_END}, {TEXT("PAGEUP"), Keyboard::KEY_PAGE_UP},
        {TEXT("PAGEDOWN"), Keyboard::KEY_PAGE_DOWN}, {TEXT("UP"), Keyboard::KEY_UP},
        {TEXT("DOWN"), Keyboard::KEY_DOWN}, {TEXT("LEFT"), Keyboard::KEY_LEFT},
        {TEXT("RIGHT"), Keyboard::KEY_RIGHT}, {TEXT("LEFTSHIFT"), Keyboard::KEY_LEFT_SHIFT},
        {TEXT("RIGHTSHIFT"), Keyboard::KEY_RIGHT_SHIFT}, {TEXT("LEFTCONTROL"), Keyboard::KEY_LEFT_CONTROL},
        {TEXT("RIGHTCONTROL"), Keyboard::KEY_RIGHT_CONTROL}, {TEXT("LEFTALT"), Keyboard::KEY_LEFT_ALT},
        {TEXT("RIGHTALT"), Keyboard::KEY_RIGHT_ALT}
    };
    if (const Keyboard* Found = Special.Find(Name)) { OutKey = *Found; return true; }
    if (Name.Len() >= 2 && Name[0] == TEXT('F'))
    {
        const int32 Number = FCString::Atoi(*Name.RightChop(1));
        if (Number >= 1 && Number <= 24)
        {
            OutKey = static_cast<Keyboard>(static_cast<int32>(Keyboard::KEY_F1) + Number - 1);
            return true;
        }
    }
    return false;
}

static bool TryMossMouseButton(const FKey& Key, Mouse& OutButton)
{
    const FName Name = Key.GetFName();
    if (Name == EKeys::LeftMouseButton.GetFName()) { OutButton = Mouse::LEFT; return true; }
    if (Name == EKeys::RightMouseButton.GetFName()) { OutButton = Mouse::RIGHT; return true; }
    if (Name == EKeys::MiddleMouseButton.GetFName()) { OutButton = Mouse::MIDDLE; return true; }
    if (Name == EKeys::ThumbMouseButton.GetFName()) { OutButton = Mouse::BUTTON_4; return true; }
    if (Name == EKeys::ThumbMouseButton2.GetFName()) { OutButton = Mouse::BUTTON_5; return true; }
    return false;
}
#endif

bool FMossInput::IsNativeAvailable() { return MOSSUNREAL_NATIVE_API_AVAILABLE != 0; }
bool FMossInput::IsKeyDown(const FKey& Key)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Keyboard NativeKey; return TryMossKeyboardKey(Key, NativeKey) && Moss_IsKeyPressed(NativeKey);
#else
    return false;
#endif
}
bool FMossInput::IsKeyReleased(const FKey& Key)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Keyboard NativeKey; return TryMossKeyboardKey(Key, NativeKey) && Moss_IsReleased(NativeKey);
#else
    return false;
#endif
}
bool FMossInput::WasKeyJustPressed(const FKey& Key)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Keyboard NativeKey; return TryMossKeyboardKey(Key, NativeKey) && Moss_IsKeyJustPressed(NativeKey);
#else
    return false;
#endif
}
bool FMossInput::WasKeyJustReleased(const FKey& Key)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Keyboard NativeKey; return TryMossKeyboardKey(Key, NativeKey) && Moss_IsKeyJustReleased(NativeKey);
#else
    return false;
#endif
}
bool FMossInput::IsMouseButtonDown(const FKey& Button)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Mouse NativeButton; return TryMossMouseButton(Button, NativeButton) && Moss_IsMousePressed(NativeButton);
#else
    return false;
#endif
}
bool FMossInput::WasMouseButtonJustPressed(const FKey& Button)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Mouse NativeButton; return TryMossMouseButton(Button, NativeButton) && Moss_IsMouseJustPressed(NativeButton);
#else
    return false;
#endif
}
bool FMossInput::WasMouseButtonJustReleased(const FKey& Button)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Mouse NativeButton; return TryMossMouseButton(Button, NativeButton) && Moss_IsMouseJustReleased(NativeButton);
#else
    return false;
#endif
}
FVector2D FMossInput::GetMousePosition()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    int X = 0, Y = 0; Moss_GetMousePosition(&X, &Y); return FVector2D(X, Y);
#else
    return FVector2D::ZeroVector;
#endif
}
void FMossInput::SetMousePosition(const FVector2D& Position)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Moss_SetMousePosition(FMath::RoundToInt(Position.X), FMath::RoundToInt(Position.Y));
#endif
}
void FMossInput::SetMouseVisible(bool bVisible)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Moss_SetMouseVisible(bVisible);
#endif
}
