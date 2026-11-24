# Moss Physics crate Actor example

This example demonstrates the recommended component layout for the direct Moss rigid-body integration:

```text
AMossPhysicsCrateActor
└── UMossPhysicsBodyComponent (root; Moss owns its dynamic transform)
    ├── UStaticMeshComponent (visual only; NoCollision)
    └── UBoxComponent (QueryOnly; imported as Moss collision geometry)
```

Add the source files to an Unreal C++ project module that depends on `MossUnreal`. Assign a static mesh to `VisualMesh` in a Blueprint derived from the example Actor. Configure `MossBody` with a Dynamic mode, the desired mass/material/DOF settings and a collision layer.

The sample collider is QueryOnly so Unreal scene queries can still use it, but Chaos does not simulate the same primitive. The actual dynamic simulation is owned by the per-world Moss `PhysicsSystem`.

Call `Launch(Direction, ImpulseNewtonSeconds)` from Blueprint or C++ after the Moss body has been created. `IsMossBodyReady()` is false until the native Moss Physics feature has been enabled and the world has successfully created the body.

**Build note:** the supplied Moss Physics headers currently have upstream C++17 compilation blockers. This example compiles only in a build where `MOSS_ENABLE_PHYSICS=1` has been enabled after the exact Moss Physics headers and library pass the standalone compile/link check described in `Docs/MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md`.
