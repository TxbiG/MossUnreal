#include "Core/Core.h"
#include "Misc/Paths.h"

/** Example entry point callable from any native C++ module; no Actor is required. */
void RunMossUnrealNativeUtilitiesExample()
{
    const FVector Direction = FMossMath::SafeNormalize(FVector(3.0, 4.0, 0.0));
    const double Distance = FMossMath::Remap(Direction.Size(), 0.0, 1.0, 0.0, 100.0, true);

    FMossDataStore State;
    State.SetString(TEXT("session"), FMossUtilities::MakeUniqueId());
    State.SetNumber(TEXT("distance"), Distance);
    State.SetBool(TEXT("initialized"), true);

    FString Error;
    State.SaveToFile(FPaths::ProjectSavedDir() / TEXT("Moss/ExampleState.json"), Error);
    if (!Error.IsEmpty())
    {
        FMossDiagnostics::Warning(Error);
    }
}
