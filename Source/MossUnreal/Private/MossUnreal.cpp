#include "MossUnreal.h"

#include "Diagnostics/MossDiagnostics.h"

#define LOCTEXT_NAMESPACE "FMossUnrealModule"

void FMossUnrealModule::StartupModule()
{
    FMossDiagnostics::Log(TEXT("MossUnreal module loaded. Register a native Moss backend provider before runtime initialization."));
}

void FMossUnrealModule::ShutdownModule()
{
    // UEngineSubsystem owns runtime shutdown after worlds have released their native worlds.
}

IMPLEMENT_MODULE(FMossUnrealModule, MossUnreal)

#undef LOCTEXT_NAMESPACE
