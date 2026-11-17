#include "Navigation/NavigationWorld.h"
bool FMossNavigationWorld::Initialize(FMossNavigationMesh& Mesh, FString& OutError)
{
    Shutdown();
    NativeMesh = Mesh.GetNativeMesh();
    if (!NativeMesh) { OutError = TEXT("Navigation mesh is not initialized."); return false; }
    OutError = TEXT("Native Moss navigation query is unavailable until Moss/Moss_Navigation.h is repaired.");
    NativeMesh = nullptr; return false;
}
void FMossNavigationWorld::Shutdown() { NativeQuery = nullptr; NativeMesh = nullptr; }
