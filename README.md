[![Source validation](https://github.com/TxbiG/MossUnreal/actions/workflows/build.yml/badge.svg)](https://github.com/TxbiG/MossUnreal/actions/workflows/build.yml)

# MossUnreal

**MossUnreal integrates the public Moss C++ framework APIs with Unreal Engine while keeping ordinary code independent of Actors, Components and Blueprints.** It provides a code-first layer, selected direct Moss adapters, and optional Unreal lifecycle/editor-facing integrations.

> **Version 0.4.0 — native-header integration pass.** The plugin now uses the actual public headers from the supplied Moss checkout for math, timers, JSON, platform/input, audio and networking, with optional GUI and XR adapters. Physics and Recast navigation are not enabled against this Moss revision because their public header families currently fail standalone compilation. The Unreal-facing physics and advanced physics component APIs remain provider-backed integration surfaces, not a claim that native simulation is already connected.

## Design goals

- **Code-first:** use Moss types and functionality from ordinary C++ classes and functions without requiring Actors or Components.
- **Moss as the source of truth:** adapters call declarations from the actual public Moss headers; no duplicate guessed `Moss::` API is introduced.
- **Unreal-native where useful:** optional subsystems, Actor Components, UMG and Blueprint integration.
- **Explicit ownership:** RAII wrappers own native audio, network, input and XR handles; Unreal owns UObjects and engine lifecycle.
- **Incremental adoption:** existing Unreal workflows and Chaos are not globally replaced.
- **Honest feature boundaries:** header/build blockers and upstream API limitations are documented instead of silently emulated.

## Native Moss modules used

| Moss public headers | MossUnreal integration |
|---|---|
| `Moss/Variants/Vector/*`, `Quat.h`, `Matrix/Mat44.h` | `FMossNativeMath`, vector/quaternion conversions and XR view conversion |
| `Moss/Resources/Timer.h` | `FMossNativeTimer` RAII wrapper |
| `Moss/Resources/Json.h` | `FMossResources` parsing and compact JSON serialization (object key order is unspecified) |
| `Moss/Moss_Platform.h` | `FMossPlatform`, keyboard/mouse and gamepad access |
| `Moss/Moss_Audio.h` | `FMossAudioSystem`, 2D/3D streams and ray-audio callbacks |
| `Moss/Moss_Network.h` | `FMossNetworkHost`, host/peer/packet/event lifecycle |
| `Moss/Moss_GUI.h` | Optional `FMossGUI` immediate-mode functions; caller must own a valid Moss GUI context/frame |
| `Moss/Moss_XR.h` | Optional XR instance/session/view/action wrappers; requires a genuine Moss renderer |

The selected headers are aggregated by `Source/MossUnreal/Public/Core/MossNative.h`. The broad `Moss/Moss.h` umbrella is deliberately not included in Unreal translation units because it exposes Moss global container names that can collide with Unreal's global container templates. See [Native header audit](Docs/NATIVE_HEADER_AUDIT.md).

## Plugin capabilities

| Area | Included classes/features | Native Moss status |
|---|---|---|
| Core/runtime | `FMossRuntime`, `IMossBackend`, runtime subsystem and ABI validation | Provider contract; provider must be built against exact Moss version |
| Code-first math/utilities | `FMossMath`, `FMossNativeMath`, `FMossUtilities`, `FMossNativeTimer`, diagnostics | Native math and timer use Moss headers when native API is configured; local convenience functions remain available without it |
| Data/resources | `FMossDataStore`, `FMossResources` | Unreal JSON interchange plus Moss `Json` parser and platform file APIs |
| Systems | `IMossSystem`, `FMossSystemRegistry` | Plain C++ systems; no UObject requirement |
| Platform/input | `FMossPlatform`, `FMossInput`, `FMossInputDevice`, `FMossInputActionMap` | Direct Moss platform/input calls when linked; this input API polls Moss's own platform state and is not a replacement for Unreal Enhanced Input |
| Audio | audio system, 2D/3D stream components, ray-audio components | Direct Moss audio calls where supported by the supplied declarations; see known signature limitations |
| Networking | `FMossNetworkHost` and packet/event wrappers | Direct Moss network API |
| GUI | `FMossGUI` immediate-mode wrapper | Optional; caller supplies active GUI context/frame |
| XR | `FMossXRSystem`, `FMossXRController` | Optional; Moss renderer required; Unreal RHI is not cast to Moss renderer |
| Physics | world/body subsystem and primitive/character/soft-body/vehicle/wheel component surfaces | Requires a separately implemented provider; upstream physics headers currently fail compile checks |
| Navigation | mesh/world/agent integration surfaces | Disabled pending repairs to upstream Recast/navigation public headers |
| Blueprint | runtime, input, diagnostics and supported utility nodes | Optional interface to stable Unreal-facing operations |

## Quick start

1. Copy this plugin folder into `YourProject/Plugins/MossUnreal/`.
2. Configure the native Moss checkout and built library before launching UnrealBuildTool:

   ```powershell
   $env:MOSS_INCLUDE_DIR = "<Moss-checkout>\include"
   $env:MOSS_LIBRARY_FILE = "<Moss-build>\<configuration>\<Moss-library-or-import-library>"
   $env:MOSS_EXTRA_LIBRARIES = "<optional-dependency-1>;<optional-dependency-2>"
   ```

   `MOSS_INCLUDE_DIR` must contain the `Moss/` directory. `MOSS_LIBRARY_FILE` must be the matching library for the active target/platform/configuration. Static builds may require `MOSS_EXTRA_LIBRARIES`.
3. If the Moss library was built with the Vulkan renderer, set `MOSS_GRAPHICS_VULKAN=1` and configure `MOSS_VULKAN_INCLUDE_DIR` to the folder containing `vulkan/vulkan.h`, or set `VULKAN_SDK`. For other renderer builds, set the matching `MOSS_GRAPHICS_*` option instead.
4. Enable optional native GUI/XR declarations only when required:

   ```powershell
   $env:MOSS_ENABLE_GUI = "1"
   $env:MOSS_ENABLE_XR = "1"
   ```

   Both options require the native Moss headers/library. Vulkan headers are needed only when the selected renderer is Vulkan. XR still requires a real `Moss_Renderer*`; it does not automatically bridge Unreal RHI.
5. Generate project files/build the Unreal project. Include `Core/Core.h` for the MossUnreal convenience API and `Core/MossNative.h` for the selected direct Moss public headers. Do not include `MossNative.h` from reflected Unreal headers; include it in `.cpp` files or non-reflected C++ headers after the build dependencies are configured.
6. For provider-backed physics/world functionality, build a provider implementing `IMossBackend` against a Moss revision whose physics public headers compile. Register the provider factory during module startup. See [Backend provider guide](Docs/BackendProvider.md).

Without native build configuration, code-first Unreal helpers remain usable, while direct Moss wrappers report unavailable or return safe failure values. No fake native simulation is substituted.

## Repository layout

```text
MossUnreal/
├── MossUnreal.uplugin
├── Source/
│   ├── MossUnreal/          # Runtime, direct Moss wrappers and Unreal adapters
│   └── MossUnrealTests/     # Editor automation tests
├── Docs/                    # Architecture, native header audit and API guides
├── Examples/                # Native C++, systems and Blueprint workflows
└── Scripts/                 # Source validation and native-header smoke checks
```

## Requirements and validation

- Unreal Engine 5.4+ is the initial target; minor versions/platforms still need validation in an Unreal installation.
- C++17.
- A matching built Moss library is required for direct native linking.
- GUI/XR require Moss build artifacts and the matching renderer build configuration. Vulkan headers are required only when the selected renderer is Vulkan.

Run `python3 Scripts/validate_plugin.py` for repository/source checks. Run `python3 Scripts/validate_moss_headers.py` with `MOSS_INCLUDE_DIR` configured to compile-check the selected public header set. These checks do not replace UnrealBuildTool/UHT, editor automation tests or link/package tests in the actual target toolchain.

## Documentation

- [Architecture](Docs/ARCHITECTURE.md)
- [Native C++ API](Docs/NativeAPI.md)
- [Native class-to-header map](Docs/NativeClassMap.md)
- [Native header audit and upstream blockers](Docs/NATIVE_HEADER_AUDIT.md)
- [Backend provider implementation](Docs/BackendProvider.md)
- [Unreal integration](Docs/UnrealIntegration.md)
- [Implementation status](Docs/IMPLEMENTATION_STATUS.md)
- [Roadmap](Docs/ROADMAP.md)
- [Native C++ examples](Examples/NativeCpp/README.md)
- [Direct Moss API example](Examples/NativeCpp/DirectMossApiExample.md)
- [Blueprint example](Examples/UnrealBlueprint/README.md)
