# Unreal-native workflow

1. Enable **Moss Unreal** and install/load a compatible native Moss backend provider.
2. Add `MossPhysicsBodyComponent` to an Actor.
3. Select Static, Kinematic or Dynamic body mode and Box, Sphere or Capsule shape.
4. Configure mass, damping and shape dimensions.
5. Use `Is Moss Runtime Available`, `Get Moss Runtime Status`, `Moss Log`, `Moss Make Unique Id`, `Moss Remap`, `Moss Clamp Delta Seconds`, and `Moss Raycast` from `Moss Blueprint Library`.

The component is optional. It is an adapter for Unreal Actors, not the required way to access Moss.

The world subsystem creates one native Moss world per `UWorld`, advances it with a fixed timestep, sends kinematic transforms to the backend and reads dynamic transforms back. Chaos is not modified or globally replaced.
