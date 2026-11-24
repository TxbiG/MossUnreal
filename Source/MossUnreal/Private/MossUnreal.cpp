#include "MossUnreal.h"

#include "Diagnostics/MossDiagnostics.h"

#define LOCTEXT_NAMESPACE "FMossUnrealModule"

void FMossUnrealModule::StartupModule()
{
    FMossDiagnostics::Log(TEXT("MossUnreal module loaded. Moss Physics uses a direct per-UWorld PhysicsSystem integration when explicitly enabled; optional service backends remain independently registerable."));
}

void FMossUnrealModule::ShutdownModule()
{
    // UEngineSubsystem owns runtime shutdown after worlds have released their native worlds.
}

IMPLEMENT_MODULE(FMossUnrealModule, MossUnreal)

#undef LOCTEXT_NAMESPACE
