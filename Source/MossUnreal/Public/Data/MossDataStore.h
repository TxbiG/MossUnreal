#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Lightweight JSON-backed data container. This is an Unreal-side interchange
 * utility, not a replacement for Moss's own data types or serialization APIs.
 */
class MOSSUNREAL_API FMossDataStore final
{
public:
    FMossDataStore();

    void Reset();
    bool HasField(const FString& Field) const;
    void SetString(const FString& Field, const FString& Value);
    void SetNumber(const FString& Field, double Value);
    void SetBool(const FString& Field, bool Value);
    void SetObject(const FString& Field, const TSharedPtr<FJsonObject>& Value);
    bool GetString(const FString& Field, FString& OutValue) const;
    bool GetNumber(const FString& Field, double& OutValue) const;
    bool GetBool(const FString& Field, bool& OutValue) const;
    TSharedPtr<FJsonObject> GetObject(const FString& Field) const;

    bool ToJson(FString& OutJson, bool bPretty = true) const;
    bool FromJson(const FString& Json, FString& OutError);
    bool SaveToFile(const FString& AbsolutePath, FString& OutError, bool bPretty = true) const;
    bool LoadFromFile(const FString& AbsolutePath, FString& OutError);

    TSharedPtr<FJsonObject> GetJsonObject() const { return Root; }

private:
    TSharedPtr<FJsonObject> Root;
};
