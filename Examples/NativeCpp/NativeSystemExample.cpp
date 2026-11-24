#include "Core/Core.h"

class FExampleGameplaySystem final : public IMossSystem
{
public:
    FName GetSystemName() const override { return TEXT("ExampleGameplaySystem"); }

    bool Initialize(const FMossSystemContext&, FString& OutError) override
    {
        OutError.Reset();
        return true;
    }

    void Tick(const FMossSystemContext& Context) override
    {
        // Run application/game logic using the supplied fixed-step context.
        LastDeltaSeconds = Context.DeltaSeconds;
    }

    void Shutdown() override {}

private:
    double LastDeltaSeconds = 0.0;
};

/** An ordinary C++ system registry; no Actor, Component or UObject is required. */
void RunNativeMossSystemExample()
{
    FMossSystemRegistry Registry;
    TSharedRef<FExampleGameplaySystem, ESPMode::ThreadSafe> System =
        MakeShared<FExampleGameplaySystem, ESPMode::ThreadSafe>();

    FString Error;
    if (!Registry.RegisterSystem(System, Error) || !Registry.Initialize(Error))
    {
        FMossDiagnostics::Warning(Error);
        return;
    }

    Registry.Tick(1.0 / 60.0);
    Registry.Shutdown();
}
