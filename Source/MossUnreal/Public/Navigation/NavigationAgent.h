#pragma once
#include "CoreMinimal.h"

class FMossNavigationWorld;

struct MOSSUNREAL_API FMossNavigationPath
{
    TArray<FVector> Points;
    bool bComplete = false;
};

class MOSSUNREAL_API FMossNavigationAgent final
{
public:
    bool FindPath(const FMossNavigationWorld& World, const FVector& Start, const FVector& End,
                  FMossNavigationPath& OutPath, int32 MaxPolygons = 256, FString* OutError = nullptr) const;
    bool FindNearestPoint(const FMossNavigationWorld& World, const FVector& Position, const FVector& SearchExtents,
                          FVector& OutNearest, uint64& OutPolygonReference) const;
};
