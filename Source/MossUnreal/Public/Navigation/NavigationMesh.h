#pragma once
#include "CoreMinimal.h"

/** Tiled Recast mesh configuration matching Moss_RecastNavMeshParams. */
struct MOSSUNREAL_API FMossNavigationMeshSettings
{
    FVector Origin = FVector::ZeroVector;
    float TileWidth = 1000.0f;
    float TileHeight = 1000.0f;
    int32 MaxTiles = 256;
    int32 MaxPolygonsPerTile = 2048;
};

class MOSSUNREAL_API FMossNavigationMesh final
{
public:
    FMossNavigationMesh();
    ~FMossNavigationMesh();
    FMossNavigationMesh(const FMossNavigationMesh&) = delete;
    FMossNavigationMesh& operator=(const FMossNavigationMesh&) = delete;
    bool Initialize(const FMossNavigationMeshSettings& Settings, FString& OutError);
    void Shutdown();
    bool IsInitialized() const;
    bool AddTile(const TArray<uint8>& TileData, int32 Flags, uint64& OutTileReference, FString& OutError);
    bool RemoveTile(uint64 TileReference, FString& OutError);
    void* GetNativeMesh() const { return NativeMesh; }
private:
    void* NativeMesh = nullptr;
};
