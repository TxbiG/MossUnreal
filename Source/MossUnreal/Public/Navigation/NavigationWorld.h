#pragma once
#include "CoreMinimal.h"
#include "Navigation/NavigationMesh.h"

class MOSSUNREAL_API FMossNavigationWorld final
{
public:
    bool Initialize(FMossNavigationMesh& Mesh, FString& OutError);
    void Shutdown();
    bool IsInitialized() const { return NativeQuery != nullptr; }
    void* GetNativeMesh() const { return NativeMesh; }
    void* GetNativeQuery() const { return NativeQuery; }
private:
    void* NativeMesh = nullptr;
    void* NativeQuery = nullptr;
};
