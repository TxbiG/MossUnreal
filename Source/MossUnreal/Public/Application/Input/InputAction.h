#pragma once

#include "CoreMinimal.h"
#include "Application/Input/Input.h"

UENUM(BlueprintType)
enum class EMossInputTrigger : uint8
{
    Held,
    JustPressed,
    JustReleased
};

/** Lightweight named action map for code-first consumers. It deliberately does not depend on UInputComponent. */
class MOSSUNREAL_API FMossInputActionMap final
{
public:
    void BindKey(FName ActionName, const FKey& Key, EMossInputTrigger Trigger = EMossInputTrigger::JustPressed);
    void Unbind(FName ActionName);
    void Clear();
    bool IsTriggered(FName ActionName) const;
    TArray<FName> GetTriggeredActions() const;

private:
    struct FBinding { FKey Key; EMossInputTrigger Trigger = EMossInputTrigger::JustPressed; };
    TMap<FName, FBinding> Bindings;
};
