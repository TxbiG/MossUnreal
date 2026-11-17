#include "Application/Platform/Platform.h"
#include "Core/MossNative.h"

bool FMossPlatform::IsNativeAvailable()
{
    return MOSSUNREAL_NATIVE_API_AVAILABLE != 0;
}

FMossPlatformInfo FMossPlatform::GetPlatformInfo()
{
    FMossPlatformInfo Result;
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    Result.AvailableCpuCores = Moss_GetAvailableCPUCores();
    Result.CpuCacheLineBytes = Moss_GetCPUCacheLineSize();
    Result.SystemRamMegabytes = Moss_GetSystemRAM();
#endif
    return Result;
}

bool FMossPlatform::OpenURL(const FString& URL)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*URL);
    return Moss_OpenURL(UTF8.Get());
#else
    return false;
#endif
}

bool FMossPlatform::CopyFile(const FString& Source, const FString& Destination, bool bOverwrite)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 Src(*Source), Dst(*Destination);
    return Moss_CopyFile(Src.Get(), Dst.Get(), bOverwrite);
#else
    return false;
#endif
}

bool FMossPlatform::CreateDirectory(const FString& Path, bool bRecursive)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Path);
    return Moss_CreateDirectory(UTF8.Get(), bRecursive);
#else
    return false;
#endif
}

bool FMossPlatform::RemovePath(const FString& Path, bool bRecursive)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Path);
    return Moss_RemovePath(UTF8.Get(), bRecursive);
#else
    return false;
#endif
}

bool FMossPlatform::RenamePath(const FString& OldPath, const FString& NewPath, bool bOverwrite)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 OldUTF8(*OldPath), NewUTF8(*NewPath);
    return Moss_RenamePath(OldUTF8.Get(), NewUTF8.Get(), bOverwrite);
#else
    return false;
#endif
}

bool FMossPlatform::GetPathInfo(const FString& Path, FMossPathInfo& OutInfo)
{
    OutInfo = FMossPathInfo();
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*Path);
    Moss_PathInfo NativeInfo{};
    if (!Moss_GetPathInfo(UTF8.Get(), &NativeInfo)) return false;
    OutInfo.SizeBytes = NativeInfo.size;
    OutInfo.bReadable = NativeInfo.readable;
    OutInfo.bWritable = NativeInfo.writable;
    OutInfo.bExecutable = NativeInfo.executable;
    return true;
#else
    return false;
#endif
}

static FString MossReadPath(bool (*Getter)(char*, int))
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    char Buffer[4096] = {};
    if (Getter(Buffer, static_cast<int>(sizeof(Buffer)))) return UTF8_TO_TCHAR(Buffer);
#endif
    return FString();
}

FString FMossPlatform::GetCurrentDirectory()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return MossReadPath(&Moss_GetCurrentDirectory);
#else
    return FString();
#endif
}

FString FMossPlatform::GetBasePath()
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    return MossReadPath(&Moss_GetBasePath);
#else
    return FString();
#endif
}

FString FMossPlatform::GetUserFolderPath(int32 NativeFolderValue)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    char Buffer[4096] = {};
    const Moss_UserFolder Folder = static_cast<Moss_UserFolder>(NativeFolderValue);
    return Moss_GetUserFolder(Folder, Buffer, static_cast<int>(sizeof(Buffer))) ? UTF8_TO_TCHAR(Buffer) : FString();
#else
    return FString();
#endif
}

FString FMossPlatform::GetPreferencePath(const FString& Organization, const FString& Application)
{
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 Org(*Organization), App(*Application);
    char Buffer[4096] = {};
    return Moss_GetPrefPath(Org.Get(), App.Get(), Buffer, static_cast<int>(sizeof(Buffer))) ? UTF8_TO_TCHAR(Buffer) : FString();
#else
    return FString();
#endif
}
