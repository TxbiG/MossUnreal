#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Physics/MossPhysicsTypes.h"
#include "MossBlueprintLibrary.generated.h"

/** Blueprint conveniences layered over the same public C++ services. */
UCLASS()
class MOSSUNREAL_API UMossBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Moss|Runtime", meta=(DisplayName="Initialize Moss Runtime"))
    static bool TryInitializeRuntime();

    UFUNCTION(BlueprintPure, Category="Moss|Runtime")
    static bool IsRuntimeAvailable();

    UFUNCTION(BlueprintPure, Category="Moss|Runtime")
    static FString GetRuntimeStatus();

    UFUNCTION(BlueprintPure, Category="Moss|Runtime")
    static FString GetBackendName();

    UFUNCTION(BlueprintCallable, Category="Moss|Diagnostics")
    static void MossLog(const FString& Message);

    UFUNCTION(BlueprintCallable, Category="Moss|Diagnostics")
    static void MossWarning(const FString& Message);

    UFUNCTION(BlueprintPure, Category="Moss|Utilities")
    static FString MossMakeUniqueId();

    UFUNCTION(BlueprintPure, Category="Moss|Math")
    static double MossRemap(double Value, double InputMinimum, double InputMaximum, double OutputMinimum, double OutputMaximum, bool bClamp = false);

    UFUNCTION(BlueprintPure, Category="Moss|Utilities")
    static float MossClampDeltaSeconds(float DeltaSeconds, float Maximum = 0.25f);

    UFUNCTION(BlueprintCallable, Category="Moss|Physics", meta=(WorldContext="WorldContextObject", DisplayName="Moss Raycast"))
    static bool MossRaycast(const UObject* WorldContextObject, FVector Start, FVector End, FMossPhysicsRaycastHit& OutHit);
};
