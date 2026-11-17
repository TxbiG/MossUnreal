#pragma once
#include "CoreMinimal.h"

struct MOSSUNREAL_API FMossJsonValidation
{
    bool bValid = false;
    FString SerializedJson;
    FString Error;
};

/** Moss JSON stringification and platform-path operations, backed by Moss/Resources/Json.h and Moss_Platform.h. JSON object key order is not guaranteed by the framework. */
class MOSSUNREAL_API FMossResources final
{
public:
    static bool IsNativeAvailable();
    static FMossJsonValidation ValidateAndSerializeJson(const FString& JsonText);
    static bool LoadJsonFile(const FString& FilePath, FString& OutSerializedJson, FString& OutError);
    static bool SaveJsonFile(const FString& FilePath, const FString& JsonText, FString& OutError);
    static bool CopyResource(const FString& Source, const FString& Destination, bool bOverwrite = false);
    static bool RemoveResource(const FString& Path, bool bRecursive = false);
};
