# MossUnreal architecture

## Design goal

MossUnreal is both a programming API and an optional Unreal Engine adapter. A developer must be able to use its native C++ services without creating an Actor, Component, Node-like object or other Unreal gameplay object. Unreal-specific workflows are conveniences layered on top.

## Module layers

- **Developer API:** `FMossRuntime`, math helpers, utilities, diagnostics, JSON data store and `FMossSystemRegistry` are directly usable from C++.
- **Backend contract:** `IMossBackend` is the single boundary between the plugin and the actual Moss public API. It owns ABI validation, native allocations, handles, unit/coordinate conversion and backend error reporting.
- **Engine lifecycle:** `UMossRuntimeSubsystem` owns process-wide initialization when managed by Unreal.
- **World integration:** `UMossPhysicsWorldSubsystem` owns one native Moss world per Unreal `UWorld`, fixed-step accumulation and component synchronization.
- **Optional object adapters:** `UMossPhysicsBodyComponent` maps an Actor/component transform to a Moss body. It does not replace Chaos or alter physics for unrelated Actors.
- **Blueprint API:** `UMossBlueprintLibrary` exposes a limited set of status, diagnostics and physics-query functions.

## Backend provider boundary

The repository template does not contain the Moss Framework headers or built library. Accordingly, MossUnreal defines a verified integration contract but does not invent `Moss::...` calls. A provider module must:

1. Include the installed Moss public headers only.
2. Check the runtime/ABI version against the provider's supported range.
3. Implement `IMossBackend` and map each method to real public Moss operations.
4. Keep native handles and memory owned by Moss.
5. Translate Unreal world units and coordinate conventions explicitly.
6. Register an `FMossBackendFactory` before `UMossRuntimeSubsystem` initializes.
7. Prevent C++ exceptions or backend-specific errors from crossing the interface.

Without a registered provider, the plugin loads, its general utilities work, and runtime/physics status reports unavailable. This is intentional and is not represented as a functioning native physics backend.

## Ownership and threading

- Unreal owns UObjects, Actors, Components, subsystems and editor data.
- Moss owns its native runtime, worlds, bodies and other native allocations.
- The provider owns translations and opaque native handles.
- Runtime initialization/shutdown is process-wide; physics worlds are per UWorld.
- Backend calls currently occur on the game thread. A future async contract must define synchronization explicitly.
- World systems registered in `FMossSystemRegistry` are ticked in ascending priority order on fixed simulation steps.

## Units and coordinate systems

The backend contract receives transforms, gravity, ray endpoints and primitive dimensions in Unreal world units (centimetres and cm/s^2). Moss's public physics declarations document collision tolerances and convex radii in metres, so the provider converts length-like physics values by 0.01 on ingress and 100.0 on egress using `FMossPhysicsUnits`. Coordinate handedness, up-axis and quaternion/basis mapping must still be verified against the selected Moss build; length conversion does not determine orientation conversion.

## Physics stepping

`UMossPhysicsWorldSubsystem` uses a fixed-step accumulator and bounded catch-up substeps. If the maximum is exceeded, excess accumulated time is dropped to avoid an unbounded catch-up spiral. This policy is explicit and configurable. Dynamic bodies are read back after stepping; kinematic bodies are pushed before each step. Static bodies are initialized from their component transform.

## Chaos coexistence

Moss physics is opt-in. MossUnreal does not globally replace Chaos, create Chaos bodies for Moss components, or automatically synchronize the same object through both solvers. Projects should choose one simulation owner per body.


## Direct framework inclusion

`Core/Core.h` exposes MossUnreal's plain-C++ convenience APIs. `Core/MossNative.h` is the separate opt-in include for the actual Moss public umbrella header and requires both a configured public include root and linked Moss library. This makes the distinction explicit: the convenience APIs are implemented by MossUnreal; the native framework APIs remain the ones declared by the selected Moss checkout.
