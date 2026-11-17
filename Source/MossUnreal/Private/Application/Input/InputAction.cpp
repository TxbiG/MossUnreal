#include "Application/Input/InputAction.h"
void FMossInputActionMap::BindKey(FName ActionName, const FKey& Key, EMossInputTrigger Trigger)
{
    if (!ActionName.IsNone() && Key.IsValid()) { FBinding B; B.Key = Key; B.Trigger = Trigger; Bindings.Add(ActionName, B); }
}
void FMossInputActionMap::Unbind(FName ActionName) { Bindings.Remove(ActionName); }
void FMossInputActionMap::Clear() { Bindings.Reset(); }
bool FMossInputActionMap::IsTriggered(FName ActionName) const
{
    const FBinding* B = Bindings.Find(ActionName); if (!B) return false;
    switch (B->Trigger)
    {
        case EMossInputTrigger::Held: return FMossInput::IsKeyDown(B->Key);
        case EMossInputTrigger::JustReleased: return FMossInput::WasKeyJustReleased(B->Key);
        default: return FMossInput::WasKeyJustPressed(B->Key);
    }
}
TArray<FName> FMossInputActionMap::GetTriggeredActions() const
{
    TArray<FName> Result;
    for (const TPair<FName, FBinding>& Pair : Bindings) if (IsTriggered(Pair.Key)) Result.Add(Pair.Key);
    return Result;
}
