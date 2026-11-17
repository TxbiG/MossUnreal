#include "Application/Input/InputDevice.h"
#include "Core/MossNative.h"

FMossInputDevice::FMossInputDevice(int32 DeviceIndex) { Open(DeviceIndex); }
FMossInputDevice::~FMossInputDevice() { Close(); }
FMossInputDevice::FMossInputDevice(FMossInputDevice&& Other) noexcept : NativeHandle(Other.NativeHandle) { Other.NativeHandle = nullptr; }
FMossInputDevice& FMossInputDevice::operator=(FMossInputDevice&& Other) noexcept
{
    if (this != &Other) { Close(); NativeHandle = Other.NativeHandle; Other.NativeHandle = nullptr; }
    return *this;
}
bool FMossInputDevice::Open(int32 DeviceIndex)
{
    Close();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (DeviceIndex < 0) return false;
    NativeHandle = Moss_OpenGamepad(static_cast<Moss_GamepadID>(DeviceIndex));
#endif
    return NativeHandle != nullptr;
}
void FMossInputDevice::Close()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeHandle) Moss_CloseGamepad(static_cast<Moss_Gamepad*>(NativeHandle));
#endif
    NativeHandle = nullptr;
}
bool FMossInputDevice::IsConnected() const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return NativeHandle && Moss_GamepadConnected(static_cast<Moss_Gamepad*>(NativeHandle));
#else
    return false;
#endif
}
FString FMossInputDevice::GetName() const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    if (NativeHandle) { const char* Name = Moss_GetGamepadName(static_cast<Moss_Gamepad*>(NativeHandle)); return Name ? UTF8_TO_TCHAR(Name) : FString(); }
#endif
    return FString();
}
bool FMossInputDevice::IsButtonDown(int32 Value) const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return NativeHandle && Moss_IsGamepadButtonPressed(static_cast<Moss_Gamepad*>(NativeHandle), static_cast<Moss_GamepadButton>(Value));
#else
    return false;
#endif
}
bool FMossInputDevice::WasButtonJustPressed(int32 Value) const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return NativeHandle && Moss_IsGamepadButtonJustPressed(static_cast<Moss_Gamepad*>(NativeHandle), static_cast<Moss_GamepadButton>(Value));
#else
    return false;
#endif
}
bool FMossInputDevice::WasButtonJustReleased(int32 Value) const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return NativeHandle && Moss_IsGamepadButtonJustReleased(static_cast<Moss_Gamepad*>(NativeHandle), static_cast<Moss_GamepadButton>(Value));
#else
    return false;
#endif
}
float FMossInputDevice::GetAxis(int32 Value) const
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return NativeHandle ? Moss_GetGamepadAxis(static_cast<Moss_Gamepad*>(NativeHandle), static_cast<GamepadAxis>(Value)) : 0.0f;
#else
    return 0.0f;
#endif
}
int32 FMossInputDevice::GetConnectedDeviceCount()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return Moss_GetNumGamepads();
#else
    return 0;
#endif
}
void FMossInputDevice::UpdateAllDevices()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Moss_UpdateGamepads();
#endif
}
