#include "MossUnreal.h"

#define LOCTEXT_NAMESPACE "FMossUnrealModule"

void FMossUnrealModule::StartupModule()
{
    // Native Moss library loading belongs here once the distribution model,
    // ABI version check and platform binaries have been established.
}

void FMossUnrealModule::ShutdownModule()
{
    // Ensure all Moss worlds and callbacks are destroyed before unloading
    // a dynamically loaded native library.
}

IMPLEMENT_MODULE(FMossUnrealModule, MossUnreal)

#undef LOCTEXT_NAMESPACE
