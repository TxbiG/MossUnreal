# Unreal Engine integration

## Requirements

- Unreal Engine 5.4+ is the initial target stated by this template. Confirm the specific minor version and platform toolchain in CI before declaring them supported.
- C++17.
- A separately built native Moss backend provider is required for native runtime/physics features.

## Installation

Copy `MossUnreal` into `YourProject/Plugins/MossUnreal`, enable the plugin, regenerate project files and build the project.

## Runtime order

The provider should register its backend factory from its module startup. If it loads later, call `TryInitializeRuntime()` on `UMossRuntimeSubsystem` or use the Blueprint `Initialize Moss Runtime` function. Without a provider, the plugin remains loaded and code-first convenience APIs work, but provider-backed runtime/physics features report unavailable.

## Physics bodies

Add `UMossPhysicsBodyComponent` to an Actor, choose a body mode and primitive shape, and configure the dimensions in Unreal world units. The body is created at BeginPlay and destroyed at EndPlay. The component exposes its opaque backend handle value to C++ for diagnostics only; it does not expose an owning native pointer or a Blueprint-editable handle. C++ raycast results also retain the opaque native body handle, while the reflected Blueprint hit structure exposes only location, normal, distance and blocking state.

`UMossPhysicsWorldSubsystem` creates one native world per UWorld. Its default step is 1/60 second with a maximum of four substeps. It reads world gravity from Unreal. Provider implementations convert length-like physics values from Unreal centimetres to Moss physics metres (0.01 scale) using `FMossPhysicsUnits`. They must separately verify coordinate handedness, up-axis and quaternion mapping against the selected Moss revision.

## Blueprint

The Blueprint function library currently exposes runtime status, backend name, logging and raycasts. More high-level Blueprint wrappers should be added only where they make sense as stable gameplay-facing operations; the full C++ API remains available to native code.

## Testing

Run the `MossUnreal` automation tests in an Unreal Editor build. The repository CI validates plugin metadata, required source files and packaging without pretending to compile Unreal Engine. A Windows self-hosted runner with Unreal installed can enable the optional BuildPlugin job using `ENABLE_UNREAL_BUILD=true` and `UNREAL_ENGINE_ROOT`.


## Late provider loading

`UMossRuntimeSubsystem` attempts initialization during engine startup. If a provider module is loaded later, call `TryInitializeRuntime()` on the subsystem or the Blueprint `Initialize Moss Runtime` function. Per-world physics subsystems retry world creation while active; body components that begin play before the backend is ready are retained as weak pending registrations and are created when the world becomes available.

## Physics component dimensions

The primitive dimensions are authored in local component units and are multiplied by the component's absolute world scale before being passed to the backend. The initial native transform carries location and rotation with unit scale. Dynamic transform reads preserve the Unreal component's existing scale. Provider implementations should not apply scale a second time.
