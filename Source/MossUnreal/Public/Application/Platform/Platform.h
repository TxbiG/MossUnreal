#pragma once

#include "CoreMinimal.h"

struct MOSSUNREAL_API FMossPlatformInfo
{
    int32 AvailableCpuCores = 0;
    int32 CpuCacheLineBytes = 0;
    int32 SystemRamMegabytes = 0;
};

struct MOSSUNREAL_API FMossPathInfo
{
    uint64 SizeBytes = 0;
    bool bReadable = false;
    bool bWritable = false;
    bool bExecutable = false;
};

/** Thin, explicit wrappers over Moss/Moss_Platform.h. */
class MOSSUNREAL_API FMossPlatform final
{
public:
    static bool IsNativeAvailable();
    static FMossPlatformInfo GetPlatformInfo();
    static bool OpenURL(const FString& URL);
    static bool CopyFile(const FString& Source, const FString& Destination, bool bOverwrite = false);
    static bool CreateDirectory(const FString& Path, bool bRecursive = true);
    static bool RemovePath(const FString& Path, bool bRecursive = false);
    static bool RenamePath(const FString& OldPath, const FString& NewPath, bool bOverwrite = false);
    static bool GetPathInfo(const FString& Path, FMossPathInfo& OutInfo);
    static FString GetCurrentDirectory();
    static FString GetBasePath();
    static FString GetUserFolderPath(int32 NativeFolderValue);
    static FString GetPreferencePath(const FString& Organization, const FString& Application);
};
