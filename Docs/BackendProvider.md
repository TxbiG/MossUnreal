# Implementing a native Moss backend provider

This guide is for the small adapter module that connects MossUnreal to a particular built Moss Framework revision. It is deliberately separate from the engine-facing plugin so that the plugin never guesses Moss symbols or depends on private implementation headers.

## 1. Build against the real framework

Use the public `include` directory from the selected Moss checkout and the corresponding library/import library produced by that checkout's CMake build. Set these environment variables for UnrealBuildTool:

- `MOSS_INCLUDE_DIR` — path to the framework's public include root (the directory containing `Moss/`).
- `MOSS_LIBRARY_FILE` — absolute path to the library/import library for the active target and configuration.

Do not copy private Moss source files into this plugin. Keep runtime DLL/SO/dylib staging in the provider's packaging rules.

## 2. Implement the contract

Implement every `IMossBackend` method:

| Contract method | Provider responsibility |
|---|---|
| `Initialize` | Validate the linked Moss version, initialize required subsystems and report errors. |
| `Shutdown` | Release native resources in reverse order and become safe to destroy. |
| `GetInfo` | Return provider name/version, provider ABI (currently 1) and supported capabilities. |
| `CreateWorld` | Translate gravity/settings and create an owned native simulation world. |
| `DestroyWorld` | Destroy all remaining native bodies/objects and release the world. |
| `StepWorld` | Advance the native world by exactly the supplied fixed delta. |
| `CreateBody` | Map supported primitive shape, mode, mass, damping and transform to real Moss operations. |
| `DestroyBody` | Release the backend-owned body handle. |
| `SetBodyTransform` | Apply a transform using the selected kinematic/static policy. |
| `GetBodyTransform` | Read the native transform and convert it back to Unreal world units. |
| `Raycast` | Run a query and return a hit with converted point, normal, distance and opaque body handle. |

If the selected Moss revision does not support one of these operations, do not silently emulate it with unrelated functionality. Report the capability as unsupported and return a clear error from the method; a later interface revision can make optional capabilities explicit.

## 3. Register the provider

In the provider module's `StartupModule`, register its factory:

```cpp
FMossRuntime::Get().RegisterBackendFactory([]()
    -> TSharedPtr<IMossBackend, ESPMode::ThreadSafe>
{
    return MakeShared<FMyVerifiedMossBackend, ESPMode::ThreadSafe>();
});
```

Set `FMossBackendInfo::AbiVersion` to `1` for this version of the `IMossBackend` contract. This is the provider ABI, not the Moss Framework version. The provider must separately validate the Moss version it was compiled against.

The provider should register its factory from its module startup. During a controlled module unload, call `FMossRuntime::Get().UnregisterBackendFactory()` only after all worlds are released and the runtime is shut down. If the provider loads after `UMossRuntimeSubsystem` has already attempted initialization, call `UMossRuntimeSubsystem::TryInitializeRuntime()` (or the Blueprint `Initialize Moss Runtime` function) to retry. World subsystems also retry world creation while their worlds are active, and body components registered before the provider becomes ready are queued for creation.

## 4. Coordinate, unit and ownership rules

- Contract transforms, gravity and primitive dimensions are expressed in Unreal world units (centimetres, cm/s^2).
- Moss's public physics declarations annotate collision tolerance and convex radius in metres. Convert Unreal length, position, linear velocity and acceleration by `0.01` at the boundary; convert returned positions, distances and dimensions by `100.0`. `FMossPhysicsUnits` exposes these conversions.
- Confirm Moss's coordinate handedness, up axis and quaternion conventions from the exact public headers before mapping rotation. Do not infer the basis conversion from the length conversion.
- Implement conversion in one place inside the provider; never spread conversions across Actor Components and Blueprint wrappers.
- Store native pointers in provider-owned maps keyed by opaque `FMossNativeHandle::Value` identifiers. Do not expose raw pointer values to Blueprints or UObject properties.
- Invalidate mappings on body/world destruction and make teardown idempotent.
- Backend calls are game-thread-only in this contract version.
- Do not let exceptions cross the interface boundary.

## 5. Provider test checklist

- ABI mismatch is rejected before any world is created.
- Failed initialization cleans up partially initialized native subsystems.
- World/body handles are unique while alive and invalid after destruction.
- Multiple UWorld instances create independent native worlds.
- Fixed-step calls use the exact supplied delta.
- Kinematic transforms travel Unreal -> Moss correctly.
- Dynamic transforms travel Moss -> Unreal correctly.
- Raycast points, normals and distances round-trip through the coordinate conversion.
- Shutdown works after partial initialization and after worlds have already been destroyed.


## Current Moss physics API integration notes

The public Moss API exposes `PhysicsSystem::Init(...)`, `PhysicsSystem::GetBodyInterface()`, `PhysicsSystem::GetNarrowPhaseQuery()` and `PhysicsSystem::Update(...)`. Its `Init` call requires broad-phase, object-vs-broad-phase and object-layer-pair filter interfaces whose lifetimes must cover the physics system. `Update` receives a temporary allocator and job system. The provider must own those objects and translate the existing backend contract to this API. Body creation is available through `BodyInterface::CreateAndAddBody(...)`; transform reads/writes are exposed by `GetPositionAndRotation(...)` and `SetPositionAndRotation(...)`. See the public headers from the exact Moss revision being linked; do not copy assumptions from another Jolt/Moss version.
