#include "Navigation/NavigationMesh.h"
// Moss/Moss_Navigation.h is intentionally not included until its Recast public header is repaired.
FMossNavigationMesh::FMossNavigationMesh() = default;
FMossNavigationMesh::~FMossNavigationMesh() { Shutdown(); }
bool FMossNavigationMesh::Initialize(const FMossNavigationMeshSettings&, FString& OutError)
{
    OutError = TEXT("Native Moss navigation is disabled: Moss/Moss_Navigation.h currently fails a standalone public-header compile. See Docs/NATIVE_HEADER_AUDIT.md.");
    return false;
}
void FMossNavigationMesh::Shutdown() { NativeMesh = nullptr; }
bool FMossNavigationMesh::IsInitialized() const { return NativeMesh != nullptr; }
bool FMossNavigationMesh::AddTile(const TArray<uint8>&, int32, uint64& OutTileReference, FString& OutError)
{ OutTileReference = 0; OutError = TEXT("Native Moss navigation is not available in this framework revision."); return false; }
bool FMossNavigationMesh::RemoveTile(uint64, FString& OutError)
{ OutError = TEXT("Native Moss navigation is not available in this framework revision."); return false; }
