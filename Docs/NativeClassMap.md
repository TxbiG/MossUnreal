# Native class map

This is the current mapping from Moss public APIs to MossUnreal classes. Public native headers are included through `Core/MossNative.h` from implementation files, not from reflected Unreal headers.

| MossUnreal class | Moss source API | Responsibilities |
|---|---|---|
| `FMossNativeMath` | `Vec2`, `Vec3`, `Quat`, `Mat44` | FVector/FQuat conversion and Moss dot/cross/normalization/min/max/slerp operations |
| `FMossNativeTimer` | `Moss/Resources/Timer.h` | Move-only C++ timer owner with callback, tick, pause, resume, stop and reset |
| `FMossPlatform` | `Moss/Moss_Platform.h` | Machine information, path/file operations, URL opening and user/preference folders |
| `FMossInput` | `Moss/Moss_Platform.h` | Keyboard/mouse state and cursor operations, independent of Enhanced Input |
| `FMossInputDevice` | `Moss/Moss_Platform.h` | RAII gamepad handle, button/axis polling and device enumeration |
| `FMossInputActionMap` | `FMossInput` | Named key-trigger mapping for code-first gameplay |
| `FMossAudioSystem` | `Moss/Moss_Audio.h` | Audio init/update/shutdown, source loading, audio ray tracing |
| `UMossAudioStream2DComponent` | `AudioStream2D` declarations | Optional ActorComponent lifecycle wrapper for a 2D stream |
| `UMossAudioStream3DComponent` | `AudioStream3D` declarations | Optional ActorComponent lifecycle wrapper for a 3D stream's supported playback controls |
| `UMossRayAudioStream2DComponent` | `Moss_AudioRayTrace2D` | Caller-provided 2D occlusion callback adapter |
| `UMossRayAudioStream3DComponent` | `Moss_AudioRayTrace3D` | Caller-provided 3D occlusion callback adapter |
| `FMossGUI` | `Moss/Moss_GUI.h` | Available immediate-mode windows/widgets; requires caller-owned active GUI context |
| `FMossNetworkHost` | `Moss/Moss_Network.h` | Non-blocking host lifecycle, connect/send/disconnect/poll/flush and packet ownership |
| `FMossResources` | `Moss/Resources/Json.h`, `Moss/Moss_Platform.h` | Moss JSON parse/stringify plus resource file operations |
| `FMossXRSystem` | `Moss/Moss_XR.h` | XR instance/session/frame lifecycle, capabilities and view extraction; requires Moss renderer |
| `FMossXRController` | `Moss/Moss_XR.h` | Pose action ownership and pose retrieval |
| `FMossNavigationMesh` | `Moss/Moss_Navigation.h` intended API | Tiled mesh lifecycle surface; disabled while upstream header fails compilation |
| `FMossNavigationWorld` | Recast query API intended in `Moss_Navigation.h` | Query owner surface; disabled while upstream header fails compilation |
| `FMossNavigationAgent` | Recast nearest-poly/path query API intended in `Moss_Navigation.h` | Navigation query surface; disabled while upstream header fails compilation |
| `UMossPhysicsBodyComponent` and world subsystem | `Moss/Physics/PhysicsSystem.h` intended API via `IMossBackend` | Unreal transform/body adapter; requires a separately built native provider |
| `UMossCharacterComponent` | `Moss/Physics/Character/Character.h` intended API | Character motion adapter surface; disabled pending upstream header repairs/provider |
| `UMossSoftBodyComponent` | `Moss/Physics/SoftBody/SoftBody.h` intended API | Soft-body configuration/adapter surface; disabled pending upstream header repairs/provider |
| `UMossVehicleComponent`, `UMossWheelComponent` | `Moss/Physics/Vehicle/Vehicle.h` intended API | Vehicle and wheel configuration/control surfaces; disabled pending upstream header repairs/provider |

## Native build switches

The plugin defines native API availability only when both `MOSS_INCLUDE_DIR` and `MOSS_LIBRARY_FILE` are configured. GUI and XR additionally require their opt-in switches:

- `MOSS_ENABLE_GUI=1`
- `MOSS_ENABLE_XR=1`

The selected renderer macro must match the Moss library build (`MOSS_GRAPHICS_VULKAN`, `MOSS_GRAPHICS_DIRECTX`, `MOSS_GRAPHICS_OPENGL`, `MOSS_GRAPHICS_OPENGLES` or `MOSS_GRAPHICS_METAL`). Set `MOSS_VULKAN_INCLUDE_DIR=<folder containing vulkan/vulkan.h>` or `VULKAN_SDK=<SDK root>` only when using the Vulkan renderer.

Physics and Recast navigation are not enabled by build switches in this revision because their current upstream header files do not compile as a public dependency set. See [Native header audit](NATIVE_HEADER_AUDIT.md).
