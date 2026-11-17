#include "Navigation/NavigationAgent.h"
#include "Navigation/NavigationWorld.h"
bool FMossNavigationAgent::FindPath(const FMossNavigationWorld& World, const FVector&, const FVector&, FMossNavigationPath& OutPath, int32, FString* OutError) const
{
    OutPath = FMossNavigationPath();
    if (OutError) *OutError = World.IsInitialized() ? TEXT("Moss navigation query is not implemented for this header revision.") : TEXT("Navigation world is not initialized.");
    return false;
}
bool FMossNavigationAgent::FindNearestPoint(const FMossNavigationWorld& World, const FVector&, const FVector&, FVector&, uint64&) const
{ return false && World.IsInitialized(); }
