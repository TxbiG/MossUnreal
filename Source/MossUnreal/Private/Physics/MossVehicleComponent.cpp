#include "Physics/MossVehicleComponent.h"
#include "Diagnostics/MossDiagnostics.h"
UMossVehicleComponent::UMossVehicleComponent() { SetMobility(EComponentMobility::Movable); }
bool UMossVehicleComponent::SetDriverInput(float InThrottle, float InSteering, float InBrake, float InHandBrake)
{
    Throttle = FMath::Clamp(InThrottle, -1.0f, 1.0f); Steering = FMath::Clamp(InSteering, -1.0f, 1.0f);
    Brake = FMath::Clamp(InBrake, 0.0f, 1.0f); HandBrake = FMath::Clamp(InHandBrake, 0.0f, 1.0f);
    return false; // Inputs are cached for the future VehicleConstraint provider; physics header is not yet compilable.
}
void UMossVehicleComponent::ResetNativeVehicle() { NativeVehicleHandle = 0; }
