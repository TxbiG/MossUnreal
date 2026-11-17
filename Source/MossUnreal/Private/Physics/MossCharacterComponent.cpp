#include "Physics/MossCharacterComponent.h"
#include "Diagnostics/MossDiagnostics.h"
UMossCharacterComponent::UMossCharacterComponent() { SetMobility(EComponentMobility::Movable); }
bool UMossCharacterComponent::SetDesiredVelocity(const FVector&)
{
    FMossDiagnostics::Warning(TEXT("Moss CharacterVirtual integration is blocked by unresolved duplicate/inconsistent declarations in Moss/Physics/Character/Character.h and related physics headers. See Docs/NATIVE_HEADER_AUDIT.md."));
    return false;
}
void UMossCharacterComponent::ResetNativeCharacter() { NativeCharacterHandle = 0; }
