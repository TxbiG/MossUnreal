# Moss Actor-Tag Workflow

This example uses the UnrealJolt-style actor-tag workflow. It is an alternative to adding `UMossPhysicsBodyComponent` manually.

## 1. Enable discovery

Open **Project Settings → Plugins → Moss Physics** and enable **Actor Tag Discovery**. The default tag/layer mapping is:

| Actor tag | Moss motion mode | Default object layer |
|---|---|---|
| `moss-static` | Static | `WorldStatic` |
| `moss-dynamic` | Dynamic | `WorldDynamic` |

The tag names and object-layer names are configurable in the same settings section.

## 2. Prepare the Actor

1. Add a `StaticMeshComponent`, `BoxComponent`, `SphereComponent` or `CapsuleComponent` to the Actor.
2. Ensure the asset has supported simple collision (box, sphere or capsule).
3. For a dynamic Actor, set its mesh/primitive components to Movable. The discovery code will promote the generated Moss body component to the Actor root and make dynamic primitive components Movable.
4. In the Actor's **Details → Actor → Tags**, add exactly one of `moss-static` or `moss-dynamic`.

A tagged Actor receives one `UMossPhysicsBodyComponent`; all supported owner collision components are collected into a single compound body. If the Actor already has an explicit Moss body component, discovery does not create a duplicate. An Actor with both tags is rejected and reported.

## 3. Chaos ownership

The generated body component defaults to **Disable Chaos Physics Collision on Owner**. This removes the Actor's primitives from Chaos physics collision (preserving QueryOnly queries where possible) before Moss body registration. Choose **Reject Moss Body When Chaos Physics Is Enabled** when the Actor must remain Chaos-owned; in that case Moss registration is refused. Moss and Chaos bodies are separate worlds and do not automatically collide or exchange contacts.

## 4. Runtime changes

The world subsystem scans existing Actors when its Moss Physics scene is ready and listens for Actors spawned later. After adding/removing tags at runtime, call `RefreshTaggedActors` on the world's `MossPhysicsWorldSubsystem`. This refresh adds missing tagged bodies; it does not automatically remove an already-created Moss body when its Actor tag is removed. Remove the component or destroy the Actor to unregister that body.

## 5. Runtime body controls

After the native Moss body has been created, `UMossPhysicsBodyComponent` exposes Blueprint-callable controls for:

- `AddForce`, `AddImpulse`, `AddTorque`, and linear/angular velocity.
- `SetMass`, `SetFriction`, `SetRestitution`, `SetGravityFactor`.
- `SetLinearDamping`, `SetAngularDamping`, `SetMaxLinearVelocity`, `SetMaxAngularVelocity`.
- `SetAllowSleeping`, `WakeBody`, `SetMotionQuality`, `SetSensor`.
- `SetEnhancedInternalEdgeRemoval`, `SetApplyGyroscopicForce`, `SetSolverSteps`.
- `SetCollisionLayer` using a configured Project Settings layer name.

Linear velocity is expressed in Unreal centimetres per second; mass is kilograms; forces are newtons; impulses are newton-seconds; torque is newton-metres; maximum linear velocity is metres per second; angular velocity is radians per second. Operations that Moss only supports for dynamic bodies return `false` for static or kinematic bodies.

## 6. Important current limitations

- The native Moss Physics build is compile-gated until the supplied Moss Physics public-header dependency chain builds cleanly as C++17.
- Static mesh extraction currently uses simple collision primitives; complex triangle mesh cooking is not implemented.
- Convex-hull creation is unavailable in the supplied Moss revision because the public convex shape base header is missing.
- The automatic body component is a rigid-body adapter, not skeletal ragdoll or vehicle support.
