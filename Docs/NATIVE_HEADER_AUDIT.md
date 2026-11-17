# Moss native header audit

This document records what was checked against the uploaded Moss framework checkout. It distinguishes APIs that the plugin wraps directly from upstream header groups that currently prevent a safe native adapter build.

## Header groups used by MossUnreal

`Source/MossUnreal/Public/Core/MossNative.h` is the single opt-in boundary for the native calls. It deliberately uses selected public headers instead of the broad `Moss/Moss.h` umbrella.

| Moss header | Plugin classes | Status |
|---|---|---|
| `Moss/Moss_stdinc.h` | All native adapters | Included as the base declaration/macros layer |
| `Moss/Variants/Vector/Vec2.h`, `Vec3.h`, `Vec4.h` | `FMossNativeMath`, audio ray callbacks, XR conversion | Header compile smoke-tested |
| `Moss/Variants/Quat.h`, `Moss/Variants/Matrix/Mat44.h` | `FMossNativeMath`, XR views | Header compile smoke-tested |
| `Moss/Resources/Json.h` | `FMossResources` | Used for parse and compact stringify; object key order is unspecified |
| `Moss/Resources/Timer.h` | `FMossNativeTimer` | Used for update/pause/resume/reset and callback ownership |
| `Moss/Moss_Platform.h` | `FMossPlatform`, `FMossInput`, `FMossInputDevice` | Calls public platform, filesystem, keyboard/mouse and gamepad declarations; Vulkan header is conditional on `MOSS_GRAPHICS_VULKAN` |
| `Moss/Moss_Audio.h` | `FMossAudioSystem`, stream components and ray trace adapters | Calls public audio APIs; documented signature gaps remain |
| `Moss/Moss_Network.h` | `FMossNetworkHost` | Calls public ENet-style host, peer, packet and event APIs |
| `Moss/Moss_GUI.h` | `FMossGUI` | Optional compile switch; requires Moss renderer declarations and an externally active GUI context; Vulkan headers are conditional on the selected renderer |
| `Moss/Moss_XR.h` | `FMossXRSystem`, `FMossXRController` | Optional compile switch; requires an actual `Moss_Renderer*`, not an Unreal RHI object |

### Why the Moss umbrella is not included

`Moss/Moss.h` includes Moss's own global `TArray`, `TMap` and `TSet` declarations. Unreal also declares global container templates with those names. Including the broad umbrella in a translation unit that has already included `CoreMinimal.h` can cause global type-name collisions. MossUnreal includes the narrower native headers it actually consumes. A temporary `TArray` token rename is scoped around each selected Moss header group that may transitively use that alias; it is removed before Unreal code is parsed. It does not rename or wrap the native exported functions or the Vec/Quat types used by the adapter.

## Known upstream blockers

### Physics

The uploaded physics public header family is not currently safe to include from the plugin. A C++17 public-header compile check against `Moss/Physics/PhysicsSystem.h`, the high-level `Moss/Moss_Physics.h` and advanced physics headers produced errors including:

- Missing include target `Moss/Physics/Collision/Shape/Shape.h` (the checkout contains `Shapes.h`).
- Duplicate declarations between high-level and low-level physics headers, including body/shape enums and material declarations.
- Incomplete or missing types required by `PhysicsSystem.h`, including settings/update-context and constraint-related types.
- Inconsistent declarations in the Character, SoftBody and Vehicle header groups (including duplicated helper classes and missing/mismatched ID/motion declarations).

For that reason, this version does not claim a concrete native `PhysicsSystem` provider. The Unreal-side physics world/body/component classes and advanced Character/SoftBody/Vehicle/Wheel component surfaces remain available, but their native operations fail explicitly until those Moss headers can be made compile-clean and then mapped to the provider contract.

### Navigation

`Moss/Moss_Navigation.h` currently fails standalone public-header compilation in this checkout. The compiler reports an unmatched `#endif` at the end of the file, as well as missing Recast context/types and declaration inconsistencies elsewhere in the header. `FMossNavigationMesh`, `FMossNavigationWorld` and `FMossNavigationAgent` therefore return a descriptive unsupported status instead of manufacturing a fake navmesh/query implementation.

### Audio declaration limitations

Moss's 3D stream creation, playback, stop, volume, pitch, loop and removal declarations have usable parameters and are wrapped. The current declarations for 3D stream position, velocity and maximum-distance setters do not accept the vector/distance values their names imply (the declarations take only a stream pointer). The plugin does not call these malformed declarations and does not claim spatial source movement works. The ray-trace API can be used when a valid native `PhysicsSystem*` and caller raycast callback are supplied.

### GUI and XR

- Moss's GUI context creation/get/set declarations are commented out in the supplied `Moss_GUI.h`. `FMossGUI` wraps the available immediate-mode widgets but requires a GUI context/frame already owned by the caller. `IsContextManagementAvailable()` consequently returns `false`.
- Moss XR initialization requires `Moss_Renderer*`. Unreal's `FRHICommandList`, `FRHITexture` or renderer interfaces are not silently cast to this pointer. An integration must supply a genuine Moss renderer and separately validate swapchain/compositor ownership before shipping.

### Other Moss resource headers reviewed but not exposed

`FMossResources` currently uses only `Resources/Json.h` (plus the platform file APIs). Other resource headers in the uploaded checkout were checked separately and are not safe to include in the plugin until their upstream declarations are repaired:

- `Resources/Config.h` includes `Moss/Variants/Vector.h`, which is not present in the supplied include tree.
- `Resources/Gradient.h` includes `Moss/Core/TArray.h`, which is not present in the supplied include tree.
- `Resources/Tween.h` fails C++17 header compilation because it uses `std::any` without including `<any>` and uses scoped enum members as unqualified default arguments/switch labels.
- `Resources/BezierCurve.h` declares `BezierCurve2` twice; the Float2 implementation also uses a non-existent `Float2(0)` constructor and scalar-left-multiply expressions that are not declared by the included Float2 header.

These findings are why the current resource adapter is limited to JSON parsing/stringification and platform file operations. The plugin does not silently replace Moss Config, Tween, Gradient or Bezier functionality with a different implementation.

## Compiler smoke tests performed

A C++17 syntax-only test included the plugin's native boundary with a predeclared dummy global `TArray` template (to simulate Unreal's global name) and called representative `Vec3`, `Quat`, `Json`, `Timer`, platform, audio and network declarations. It compiled successfully in this environment. Enabling the GUI and XR header switches also passed the syntax-only test with the current renderer configuration. The compiler emitted an upstream warning for an inline `Mat33` constructor declaration without a visible definition; it was not an error in this smoke test.

This test proves public-header syntax only. It does not prove that a Moss library binary is built, that all platform-specific symbols link, or that UnrealBuildTool/UHT compiles the plugin. A matching Moss binary and Unreal Engine toolchain are still required for those checks. Run `Scripts/validate_moss_headers.py` to repeat the header check against a local checkout; set `MOSS_ENABLE_GUI` and `MOSS_ENABLE_XR` to include those optional headers in the test.
