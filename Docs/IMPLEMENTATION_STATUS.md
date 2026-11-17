# MossUnreal implementation status

Version: 0.4.0

This release is the first pass that has the actual supplied Moss Framework checkout available and uses its public headers as the implementation basis for supported module adapters.

## Delivered in this pass

1. **Native header boundary:** selected public Moss module headers are integrated through `Core/MossNative.h`. The broad `Moss/Moss.h` umbrella is intentionally avoided in Unreal translation units because it exposes global Moss container declarations that conflict with Unreal container names.
2. **Code-first math and timer:** `FMossNativeMath` calls Moss `Vec3` and `Quat` methods. `FMossNativeTimer` owns the framework's `Timer` class and forwards tick/control/callback operations.
3. **Platform and input:** `FMossPlatform`, `FMossInput` and `FMossInputDevice` call the public platform, keyboard/mouse and gamepad APIs.
4. **Audio:** `FMossAudioSystem` and stream/ray-audio components use the current public audio declarations for initialization, stream creation, playback controls, update and supported ray callbacks.
5. **Networking:** `FMossNetworkHost` wraps the Moss network host/peer/packet/event API with non-blocking polling and host reference-counted network initialization.
6. **Resources/data:** `FMossResources` calls Moss `Json` parse/stringify and platform file functions; the framework uses an unordered object map, so serialized object-key ordering is not canonical or guaranteed. `FMossDataStore` remains the separate Unreal JSON helper.
7. **GUI:** optional `FMossGUI` wrappers call the available immediate-mode widget declarations, requiring the caller to own the context/frame.
8. **XR:** optional `FMossXRSystem` and `FMossXRController` wrappers call the current session/view/action APIs, requiring a genuine Moss renderer.
9. **Physics/provider:** the Unreal world/body/backend contract and character/soft-body/vehicle/wheel component surfaces remain present; the native provider is not implemented because the supplied Moss physics public headers do not compile cleanly as a standalone dependency set.
10. **Navigation:** Unreal-side adapter surfaces remain, but native Recast integration is disabled because `Moss_Navigation.h` fails standalone header compilation.

## Source-derived constraints

- `Moss/Moss.h` is not used by the plugin's Unreal-facing native boundary. It imports global Moss `TArray`, `TMap` and `TSet` declarations which collide with Unreal's global container templates.
- Selected headers are included individually. The Float vector headers pull in a global `TArray` alias; `MossNative.h` temporarily renames that token while each selected Moss header group is parsed, then restores the normal token.
- The Moss platform/renderer headers include Vulkan declarations only when `MOSS_GRAPHICS_VULKAN` is enabled; set the matching renderer build macro and SDK include path when using that backend.
- The current GUI header does not expose active context creation/get/set declarations. Its SliderFloat/SliderInt declarations take the value by copy; the wrapper's DragFloat/DragInt helpers are used when edited values are required.
- The XR API requires a `Moss_Renderer*`; an Unreal RHI interface is not silently converted into one. Global XR initialization is reference-counted across wrapper instances and enforces one shared renderer pointer while in use.
- The current 3D audio position/velocity/max-distance declarations do not accept their nominal values; the plugin avoids calling these signatures.
- The physics public include family currently has missing include paths, duplicate declarations and incomplete/mismatched types. The navigation public header has an unmatched preprocessor guard and missing/inconsistent Recast declarations.

## Validation performed

- Source repository checks: `python3 Scripts/validate_plugin.py`.
- Python validator syntax: `python3 -m py_compile Scripts/validate_plugin.py`.
- Native header C++17 syntax smoke test against the uploaded Moss include tree, including optional GUI/XR headers and a simulated pre-existing global Unreal `TArray` declaration.
- Plugin descriptor JSON and ZIP integrity checks.

## Not validated in this environment

- UnrealBuildTool/UHT compile and Unreal Editor automation tests: Unreal Engine toolchain not available in this environment.
- Final native linking and runtime deployment: matching target/platform Moss libraries and transitive dependencies must be built/configured by the consuming project.
- End-to-end audio, network, GUI and XR runtime behavior: requires platform devices, service setup and runtime integration.
- Native physics simulation and Recast navigation: blocked by the current public header compile failures described above.

See [Native header audit](NATIVE_HEADER_AUDIT.md) and [Native class map](NativeClassMap.md) for class-to-header coverage and exact caveats.
