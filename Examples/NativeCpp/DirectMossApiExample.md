# Direct Moss API example

`DirectMossApiExample.cpp` demonstrates direct use of the real Moss public API from an ordinary Unreal C++ translation unit. It includes `Moss/Variants/Vector/Vec3.h` and uses the declared `Vec3(float, float, float)`, `Normalized()` and `Dot()` members, then shows direct `PhysicsSystem` access through `GetBodyInterface()` and `GetNarrowPhaseQuery()`. None of these operations requires an Actor, Component, UObject or Blueprint.

The physics example intentionally accepts an already-owned `PhysicsSystem` rather than creating one: Moss physics initialization requires caller-owned broad-phase/filter interfaces and an update job/temporary-allocation setup, which must be configured by the application that owns the simulation. The selected Moss public headers and library must match the exact build being linked.

Build this example only after configuring `MOSS_INCLUDE_DIR` and `MOSS_LIBRARY_FILE` for the selected Moss build. The source deliberately uses the real public declarations rather than inventing a MossUnreal wrapper or a guessed `Moss::` namespace.
