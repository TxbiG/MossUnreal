# MossUnreal

Native Unreal Engine integration for the [Moss Framework](https://github.com/TxbiG/Moss).

> **Status: Foundation / not yet connected to Moss physics.**
>
> This repository establishes the Unreal plugin structure, module lifecycle, world subsystem,
> component entry point, documentation and CI scaffolding. It deliberately does not invent or
> call Moss APIs. The native adapter must be implemented against the actual public Moss headers.

## Goals

- Provide an Unreal-native workflow for creating and managing a Moss-owned physics world.
- Keep Moss simulation ownership separate from Chaos.
- Support explicit per-world lifecycle and deterministic stepping policy.
- Expose physics objects through Unreal components once the public Moss API has been audited.
- Allow projects to adopt Moss incrementally rather than requiring a global engine physics replacement.

## Repository layout

```text
MossUnreal/
├── MossUnreal.uplugin
├── Source/
│   ├── MossUnreal/
│   │   ├── Public/
│   │   └── Private/
│   └── MossUnrealEditor/
├── Config/
├── Docs/
├── Tests/
└── .github/workflows/
```

## Initial integration model

`UMossPhysicsWorldSubsystem` is a per-`UWorld` owner for the future native Moss world.
`UMossPhysicsBodyComponent` is the Unreal-facing component entry point.

The current implementation is intentionally a lifecycle shell. It does not create a native
world, body, collider, or shape until those operations have been mapped to verified public
Moss APIs.

## Requirements

- Unreal Engine 5.4+ (initial target; validate against the exact engine versions you support).
- C++17 toolchain supported by the selected Unreal version.
- Moss Framework built as a compatible native library when the adapter milestone begins.

## Install

Copy this repository's `MossUnreal` plugin folder into:

```text
YourProject/Plugins/MossUnreal/
```

Enable **Moss Unreal** in the Unreal Editor and regenerate project files if required.

## Build status

The plugin module is a structural starting point. A full build should be validated inside a
matching Unreal Engine installation; this repository does not include Unreal Engine headers or
binaries.

## Roadmap

1. Audit Moss public C/C++ headers and identify supported physics operations.
2. Implement a versioned native adapter around those verified operations.
3. Add per-world native simulation creation, fixed-step ticking and teardown.
4. Add body and collider creation, transform synchronisation and query support.
5. Add collision event marshalling, debug drawing and automation tests.
6. Add packaging and supported-engine-version CI.

## Design rules

- Never access Moss private implementation details from the plugin.
- Never let Unreal own or free memory allocated by Moss (or vice versa) without an explicit
  ownership contract.
- Do not silently run Chaos and Moss on the same body.
- Make coordinate-system, unit, timestep and threading conversions explicit.
- Treat callbacks crossing the native/Unreal boundary as lifetime-sensitive.
