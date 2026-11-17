#include "Data/MossDataStore.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FMossDataStore::FMossDataStore()
    : Root(MakeShared<FJsonObject>())
{
}

void FMossDataStore::Reset()
{
    Root = MakeShared<FJsonObject>();
}

bool FMossDataStore::HasField(const FString& Field) const
{
    return Root.IsValid() && Root->HasField(Field);
}

void FMossDataStore::SetString(const FString& Field, const FString& Value)
{
    Root->SetStringField(Field, Value);
}

void FMossDataStore::SetNumber(const FString& Field, const double Value)
{
    Root->SetNumberField(Field, Value);
}

void FMossDataStore::SetBool(const FString& Field, const bool Value)
{
    Root->SetBoolField(Field, Value);
}

void FMossDataStore::SetObject(const FString& Field, const TSharedPtr<FJsonObject>& Value)
{
    if (Value.IsValid())
    {
        Root->SetObjectField(Field, Value);
    }
    else
    {
        Root->RemoveField(Field);
    }
}

bool FMossDataStore::GetString(const FString& Field, FString& OutValue) const
{
    return Root.IsValid() && Root->TryGetStringField(Field, OutValue);
}

bool FMossDataStore::GetNumber(const FString& Field, double& OutValue) const
{
    return Root.IsValid() && Root->TryGetNumberField(Field, OutValue);
}

bool FMossDataStore::GetBool(const FString& Field, bool& OutValue) const
{
    return Root.IsValid() && Root->TryGetBoolField(Field, OutValue);
}

TSharedPtr<FJsonObject> FMossDataStore::GetObject(const FString& Field) const
{
    const TSharedPtr<FJsonObject>* Found = nullptr;
    return Root.IsValid() && Root->TryGetObjectField(Field, Found) && Found ? *Found : nullptr;
}

bool FMossDataStore::ToJson(FString& OutJson, const bool bPretty) const
{
    OutJson.Reset();
    if (!Root.IsValid())
    {
        return false;
    }

    if (bPretty)
    {
        const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
            TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJson);
        return FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
    }

    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
    return FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
}

bool FMossDataStore::FromJson(const FString& Json, FString& OutError)
{
    TSharedPtr<FJsonObject> Parsed;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Parsed) || !Parsed.IsValid())
    {
        OutError = TEXT("Input is not a valid JSON object.");
        return false;
    }

    Root = MoveTemp(Parsed);
    OutError.Reset();
    return true;
}

bool FMossDataStore::SaveToFile(const FString& AbsolutePath, FString& OutError, const bool bPretty) const
{
    FString Json;
    if (!ToJson(Json, bPretty))
    {
        OutError = TEXT("Could not serialize the Moss data store.");
        return false;
    }
    const FString ParentDirectory = FPaths::GetPath(AbsolutePath);
    if (!ParentDirectory.IsEmpty() && !IFileManager::Get().MakeDirectory(*ParentDirectory, true))
    {
        OutError = FString::Printf(TEXT("Could not create directory: %s"), *ParentDirectory);
        return false;
    }
    if (!FFileHelper::SaveStringToFile(Json, *AbsolutePath))
    {
        OutError = FString::Printf(TEXT("Could not write JSON file: %s"), *AbsolutePath);
        return false;
    }
    OutError.Reset();
    return true;
}

bool FMossDataStore::LoadFromFile(const FString& AbsolutePath, FString& OutError)
{
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *AbsolutePath))
    {
        OutError = FString::Printf(TEXT("Could not read JSON file: %s"), *AbsolutePath);
        return false;
    }
    return FromJson(Json, OutError);
}
