#include "Diagnostics/MossDiagnostics.h"

DEFINE_LOG_CATEGORY(LogMossUnreal);

void FMossDiagnostics::Log(const FString& Message)
{
    UE_LOG(LogMossUnreal, Log, TEXT("%s"), *Message);
}

void FMossDiagnostics::Warning(const FString& Message)
{
    UE_LOG(LogMossUnreal, Warning, TEXT("%s"), *Message);
}

void FMossDiagnostics::Error(const FString& Message)
{
    UE_LOG(LogMossUnreal, Error, TEXT("%s"), *Message);
}
