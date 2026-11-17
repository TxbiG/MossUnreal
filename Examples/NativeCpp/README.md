# Native C++ usage (no Actor or Component required)

The utility and data APIs are ordinary C++ and do not require a `UObject`, `AActor` or `UActorComponent`.

```cpp
#include "Core/Core.h"

void RunFrameworkUtilities()
{
    const double Speed = FMossMath::Remap(0.5, 0.0, 1.0, 0.0, 600.0);
    const FString Id = FMossUtilities::MakeUniqueId();

    FMossDataStore SaveData;
    SaveData.SetString(TEXT("session_id"), Id);
    SaveData.SetNumber(TEXT("target_speed"), Speed);

    FString Error;
    SaveData.SaveToFile(FPaths::ProjectSavedDir() / TEXT("Moss/session.json"), Error);
}
```

## Registering the real native Moss provider

A provider module should implement `IMossBackend` using the **actual public headers and symbols** from the Moss Framework repository. In that provider module's startup, register the concrete factory:

```cpp
FMossRuntime::Get().RegisterBackendFactory([]()
    -> TSharedPtr<IMossBackend, ESPMode::ThreadSafe>
{
    return MakeShared<FMyVerifiedMossBackend, ESPMode::ThreadSafe>();
});
```

`FMyVerifiedMossBackend` above is the provider project's own class, not a class shipped by MossUnreal. It must implement the complete `IMossBackend` contract and perform the ABI/version handshake. Do not copy guessed Moss API names into this plugin.

The provider must be loaded before `UMossRuntimeSubsystem` initializes. When using a separate Unreal module, declare its module dependency/loading order accordingly.

## Optional direct framework headers

Set `MOSS_INCLUDE_DIR` and `MOSS_LIBRARY_FILE` in the environment used by UnrealBuildTool to propagate the actual Moss public include path and link artifact through the plugin's public module rules. Then include the specific public headers supplied by your checked-out Moss revision. The correct header names and function signatures must be taken from that revision's `include/Moss` directory and API reference; this example intentionally does not invent them.


Other native examples:

- [Plain C++ system registry](NativeSystemExample.md) — no Actor, Component or UObject inheritance.
- [Direct Moss API access](DirectMossApiExample.md) — uses the actual Moss `Vec3` API and accesses an existing native `PhysicsSystem` without an Unreal gameplay object.
