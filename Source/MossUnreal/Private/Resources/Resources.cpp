#include "Resources/Resources.h"
#include "Core/MossNative.h"
#include "Misc/FileHelper.h"
#include "Application/Platform/Platform.h"

bool FMossResources::IsNativeAvailable() { return MOSSUNREAL_NATIVE_API_AVAILABLE != 0; }
FMossJsonValidation FMossResources::ValidateAndSerializeJson(const FString& JsonText)
{
    FMossJsonValidation Result;
#if MOSSUNREAL_NATIVE_API_AVAILABLE
    FTCHARToUTF8 UTF8(*JsonText);
    try
    {
        std::shared_ptr<Json> Parsed = Json::parse(std::string(UTF8.Get()));
        if (!Parsed) { Result.Error = TEXT("Moss JSON parser returned an empty document."); return Result; }
        const std::string Serialized = Parsed->stringify();
        Result.SerializedJson = UTF8_TO_TCHAR(Serialized.c_str()); Result.bValid = true;
    }
    catch (const std::exception& Error)
    {
        Result.Error = UTF8_TO_TCHAR(Error.what());
    }
#else
    Result.Error = TEXT("Moss native headers/library are not configured for this build.");
#endif
    return Result;
}
bool FMossResources::LoadJsonFile(const FString& FilePath, FString& OutSerializedJson, FString& OutError)
{
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *FilePath)) { OutError = FString::Printf(TEXT("Unable to read JSON file: %s"), *FilePath); return false; }
    FMossJsonValidation Parsed = ValidateAndSerializeJson(Source);
    if (!Parsed.bValid) { OutError = Parsed.Error; return false; }
    OutSerializedJson = MoveTemp(Parsed.SerializedJson); OutError.Reset(); return true;
}
bool FMossResources::SaveJsonFile(const FString& FilePath, const FString& JsonText, FString& OutError)
{
    FMossJsonValidation Parsed = ValidateAndSerializeJson(JsonText);
    if (!Parsed.bValid) { OutError = Parsed.Error; return false; }
    if (!FFileHelper::SaveStringToFile(Parsed.SerializedJson, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    { OutError = FString::Printf(TEXT("Unable to write JSON file: %s"), *FilePath); return false; }
    OutError.Reset(); return true;
}
bool FMossResources::CopyResource(const FString& Source, const FString& Destination, bool bOverwrite)
{
    return FMossPlatform::CopyFile(Source, Destination, bOverwrite);
}
bool FMossResources::RemoveResource(const FString& Path, bool bRecursive)
{
    return FMossPlatform::RemovePath(Path, bRecursive);
}
