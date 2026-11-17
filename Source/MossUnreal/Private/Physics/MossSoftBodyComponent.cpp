#include "Physics/MossSoftBodyComponent.h"
UMossSoftBodyComponent::UMossSoftBodyComponent() { SetMobility(EComponentMobility::Movable); }
bool UMossSoftBodyComponent::RebuildNativeSoftBody(FString& OutError)
{
    NativeSoftBodyHandle = 0;
    OutError = TEXT("Moss SoftBody integration is blocked by the current native physics header dependency chain. See Docs/NATIVE_HEADER_AUDIT.md.");
    return false;
}
void UMossSoftBodyComponent::ResetNativeSoftBody() { NativeSoftBodyHandle = 0; }
