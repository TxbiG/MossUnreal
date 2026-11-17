# Native C++ API

MossUnreal offers three layers:

1. **MossUnreal code-first helpers** — Unreal-friendly utilities and registries that do not derive from `UObject`, `AActor` or `UActorComponent`.
2. **Direct Moss APIs** — the public types and functions declared by the supplied Moss headers, consumed through small wrappers or directly by game C++.
3. **Unreal adapters** — optional subsystems, Actor Components, UMG and Blueprint functions that connect those APIs to Unreal lifecycles.

## Code-first API

Include `Core/Core.h` or individual MossUnreal headers.

| Class | Purpose |
|---|---|
| `FMossRuntime`, `IMossBackend` | Backend lifecycle, ABI handshake and provider contract |
| `FMossMath`, `FMossUtilities`, `FMossDiagnostics` | General Unreal-friendly math, identifiers, time and diagnostics |
| `FMossNativeMath` | Conversion helpers that call Moss `Vec3` and `Quat` operations when native Moss is linked |
| `FMossNativeTimer` | RAII wrapper around the Moss `Timer` class |
| `FMossDataStore` | Unreal JSON-backed key/value interchange and persistence |
| `FMossResources` | Moss JSON parsing and compact serialization and Moss platform file operations |
| `IMossSystem`, `FMossSystemRegistry` | Plain C++ system lifecycle and priority-ordered updates |
| `FMossPlatform`, `FMossInput`, `FMossInputDevice` | Platform information, keyboard/mouse polling and gamepad access |
| `FMossAudioSystem` | Native Moss audio initialization, stream operations and supported ray callbacks |
| `FMossNetworkHost` | Native Moss host, peer, packet and event operations |
| `FMossGUI` | Optional immediate-mode GUI function wrapper |
| `FMossXRSystem`, `FMossXRController` | Optional Moss XR instance/session/view/action wrappers |

Most wrappers use Unreal types at their public boundary (`FString`, `FVector`, `FQuat`, `TArray`, `TFunction`) and convert to Moss types inside `.cpp` files. That keeps Moss global declarations out of reflected Unreal headers.

## Configure Moss headers and libraries

`MossUnreal.Build.cs` reads these environment variables when UnrealBuildTool starts:

- `MOSS_INCLUDE_DIR`: absolute public include root containing `Moss/`.
- `MOSS_LIBRARY_FILE`: absolute path to the matching built Moss library/import library.
- `MOSS_EXTRA_LIBRARIES`: optional semicolon-separated list of absolute transitive library paths for static builds.
- `MOSS_VULKAN_INCLUDE_DIR`: folder containing `vulkan/vulkan.h` when the Vulkan renderer is enabled.
- `VULKAN_SDK`: alternate Vulkan SDK root; Build.cs uses its `Include` subfolder.
- `MOSS_GRAPHICS_VULKAN`, `MOSS_GRAPHICS_DIRECTX`, `MOSS_GRAPHICS_OPENGL`, `MOSS_GRAPHICS_OPENGLES`, `MOSS_GRAPHICS_METAL`: set the renderer macro used by the matching Moss build; each is opt-in and exported to dependent modules.
- `MOSS_ENABLE_GUI`: set to `1`, `true` or `yes` to enable optional Moss GUI headers/wrappers.
- `MOSS_ENABLE_XR`: set to `1`, `true` or `yes` to enable optional Moss XR headers/wrappers.

Example PowerShell setup:

```powershell
$env:MOSS_INCLUDE_DIR = "C:\src\Moss\include"
$env:MOSS_LIBRARY_FILE = "C:\src\Moss\build\Release\Moss.lib"
$env:MOSS_EXTRA_LIBRARIES = "C:\deps\some-static-dependency.lib"
$env:MOSS_VULKAN_INCLUDE_DIR = "C:\VulkanSDK\Include"
$env:MOSS_ENABLE_GUI = "1" # optional
$env:MOSS_ENABLE_XR = "1"  # optional
```

Set variables before launching the Unreal Editor or UnrealBuildTool. Both the include root and library must be set before `MOSSUNREAL_HAS_MOSS_API` becomes enabled. If only one is set, native wrappers remain disabled and Build.cs reports the incomplete configuration. If Vulkan was selected, the matching Vulkan header folder is required.

## Native header boundary

Use:

```cpp
#include "Core/MossNative.h"
```

This is a **selected-header aggregate**, not an alias for `Moss/Moss.h`. It includes the exact framework headers consumed by this plugin: `Moss_stdinc.h`, selected vector/quaternion/matrix headers, `Resources/Json.h`, `Resources/Timer.h`, `Moss_Platform.h`, `Moss_Audio.h` and `Moss_Network.h`. GUI and XR headers are included only under their explicit build switches.

Do not include `MossNative.h` from a reflected `.generated.h`-using header or from a header included by `CoreMinimal.h` unless you have deliberately checked that TU's include interactions. Moss's broad umbrella includes global `TArray`, `TMap` and `TSet`; Unreal uses the same global container names. MossUnreal therefore avoids that umbrella, and scopes a temporary `TArray` token rename around selected Moss declarations which need it. See [Native header audit](NATIVE_HEADER_AUDIT.md).

## Examples based on actual framework declarations

`Examples/NativeCpp/DirectMossApiExample.cpp` demonstrates `Vec3`, Moss platform CPU information and `Json::parse` through the current public declarations. `FMossNativeMath` wraps the current `Vec3`/`Quat` operations, and `FMossNativeTimer` owns a real `Timer` instance. `FMossPlatform`, `FMossInput`, `FMossAudioSystem`, `FMossNetworkHost` and `FMossXRSystem` call their corresponding global Moss APIs where the current headers expose usable signatures.

Moss declarations are not consistently inside a `Moss` namespace. Always use the names and signatures from the exact framework checkout being linked; do not infer namespace membership from the plugin name.

## Optional modules and limitations

- **GUI:** `Moss_GUI.h` exposes immediate-mode functions, but its context create/get/set declarations are commented out. `FMossGUI` can issue widget calls only when a caller has already created and activated a valid Moss GUI context/frame. The current SliderFloat/SliderInt functions accept values by copy, so use DragFloat/DragInt when you need an edited value back.
- **XR:** Moss XR initialization takes a genuine `Moss_Renderer*`. Unreal RHI objects are not binary-compatible substitutes. A real renderer bridge and swapchain/compositor integration are outside this adapter pass. Global XR initialization is reference-counted for multiple `FMossXRSystem` owners, and concurrent owners must use the same renderer pointer.
- **Audio:** Current 3D source position/velocity/max-distance setters in the uploaded header do not accept the values suggested by their names. The adapter does not call those unusable declarations or claim spatial movement is implemented.
- **Physics:** `PhysicsSystem` and related public headers currently fail the standalone header compile due to missing includes, duplicate declarations and incomplete/mismatched types. The plugin's physics components remain behind `IMossBackend`; a concrete provider must wait for a compile-clean Moss physics public API.
- **Navigation:** `Moss_Navigation.h` currently fails standalone compilation because of a mismatched preprocessor guard and missing/inconsistent Recast declarations. Navigation adapters report unsupported rather than constructing fake queries.

## Backend lifecycle

A native provider implements `IMossBackend` and registers its factory with `FMossRuntime`. `UMossRuntimeSubsystem` attempts initialization during engine startup; a provider loaded later can be activated using `TryInitializeRuntime()` or the Blueprint runtime initialization node. The provider ABI is MossUnreal's adapter ABI, not the Moss framework version. A provider must record and validate the exact Moss revision/build options it supports.

## Data and systems

`FMossDataStore` is an Unreal JSON interchange helper. `FMossResources` separately uses Moss's own `Json` parser and platform file declarations. Moss `Json::Object` is an `std::unordered_map`, so stringify output does not promise stable member ordering. Neither should be presented as a replacement for all Moss resource/serialization facilities.

Implement `IMossSystem` in an ordinary C++ class and register it with `FMossSystemRegistry`. Systems initialize and tick in ascending priority order, then shut down in reverse order. They do not need UObject inheritance.
