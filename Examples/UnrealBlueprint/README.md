# Unreal-native Moss Physics workflow

Moss Physics runs as an independent simulation owned by `UMossPhysicsWorldSubsystem`, one per `UWorld`. Chaos remains available for unrelated actors, but a Moss-owned dynamic body must not also be actively simulated by Chaos.

## Recommended Actor setup

1. Add `MossPhysicsBodyComponent` as the Actor's **root component**. This lets the native dynamic body drive the Actor transform without a second transform adapter.
2. Attach visual mesh components beneath the Moss body component. For visual-only meshes, set collision to `NoCollision`.
3. Add one or more `BoxComponent`, `SphereComponent` or `CapsuleComponent` children for collision. Use `QueryOnly` if you still need Unreal/Chaos scene queries; do not use Chaos simulation on the same body.
4. Alternatively, attach a `StaticMeshComponent` with authored simple collision. Moss imports its BodySetup aggregate box/sphere/capsule geometry. Convex elements are not yet constructible with the supplied Moss public headers and will produce an explicit body-creation error.
5. Configure `BodyMode` (Static, Kinematic or Dynamic), `CollisionLayer`, mass, friction, restitution, damping, gravity factor, velocity limits and allowed DOFs.
6. Enable **Project Settings > Plugins > Moss Physics > Debug Draw** to inspect the gathered primitives. Static mesh bounds in this debug view are approximate; they do not render each exact simple-collision subshape.

## Runtime controls

`MossPhysicsBodyComponent` exposes Blueprint/C++ functions for:

- `AddForce` (Newtons), `AddImpulse` (Newton-seconds), `AddTorque` (Newton-metres).
- `SetLinearVelocity` / `GetLinearVelocity` (Unreal centimetres per second).
- `SetAngularVelocity` / `GetAngularVelocity` (radians per second).
- `SetFriction`, `SetRestitution` and `SetGravityFactor`.
- `RecreateMossBody` to rebuild a body after changing authored collision/body settings.

The world subsystem exposes `Moss Raycast`, its contact-added delegate and fixed-step configuration. Contact events are queued safely from Moss's worker callbacks and broadcast on the game thread.

## Build requirement

The direct Moss Physics source is compile-gated until the exact supplied Moss Physics header chain compiles as C++17 and a matching Moss library is built. See [`MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md`](../../Docs/MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md) before enabling `MOSS_ENABLE_PHYSICS`.

Without that gate, the components remain visible in the API, but no fake Chaos simulation is substituted and `IsMossBodyReady` stays false.
