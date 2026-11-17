#pragma once

#include "CoreMinimal.h"

/** RAII handle for one Moss platform gamepad. */
class MOSSUNREAL_API FMossInputDevice final
{
public:
    FMossInputDevice() = default;
    explicit FMossInputDevice(int32 DeviceIndex);
    ~FMossInputDevice();
    FMossInputDevice(FMossInputDevice&& Other) noexcept;
    FMossInputDevice& operator=(FMossInputDevice&& Other) noexcept;
    FMossInputDevice(const FMossInputDevice&) = delete;
    FMossInputDevice& operator=(const FMossInputDevice&) = delete;

    bool Open(int32 DeviceIndex);
    void Close();
    bool IsOpen() const { return NativeHandle != nullptr; }
    bool IsConnected() const;
    FString GetName() const;
    bool IsButtonDown(int32 NativeButtonValue) const;
    bool WasButtonJustPressed(int32 NativeButtonValue) const;
    bool WasButtonJustReleased(int32 NativeButtonValue) const;
    float GetAxis(int32 NativeAxisValue) const;
    static int32 GetConnectedDeviceCount();
    static void UpdateAllDevices();

private:
    void* NativeHandle = nullptr;
};
