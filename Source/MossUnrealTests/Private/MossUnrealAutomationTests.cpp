#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Core/MossBackend.h"
#include "Core/MossRuntime.h"
#include "Data/MossDataStore.h"
#include "Math/MossMath.h"
#include "Physics/MossPhysicsUnits.h"
#include "Physics/MossPhysicsSettings.h"
#include "Systems/MossSystem.h"
#include "Utilities/MossUtilities.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossMathRemapAutomationTest,
    "MossUnreal.Core.Math.Remap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossMathRemapAutomationTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Midpoint remaps to midpoint"), FMossMath::Remap(5.0, 0.0, 10.0, 0.0, 100.0), 50.0);
    TestEqual(TEXT("Clamped remap stays within the output range"), FMossMath::Remap(20.0, 0.0, 10.0, 0.0, 1.0, true), 1.0);
    TestEqual(TEXT("Degenerate input range returns output minimum"), FMossMath::Remap(3.0, 1.0, 1.0, 7.0, 9.0), 7.0);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossPhysicsUnitsAutomationTest,
    "MossUnreal.Physics.UnitConversions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossPhysicsUnitsAutomationTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("100 Unreal centimetres convert to one Moss metre"), FMossPhysicsUnits::ToMossMeters(100.0), 1.0);
    TestEqual(TEXT("One Moss metre converts to 100 Unreal centimetres"), FMossPhysicsUnits::ToUnrealCentimeters(1.0), 100.0);
    const FVector UnrealVector(100.0, -250.0, 50.0);
    const FVector MossVector = FMossPhysicsUnits::ToMossMeters(UnrealVector);
    TestTrue(TEXT("Vector conversion remaps Z-up to Y-up and converts centimetres to metres"),
        MossVector.Equals(FVector(1.0, 0.5, 2.5), 1.0e-6));
    TestTrue(TEXT("Vector conversion round-trips"),
        FMossPhysicsUnits::ToUnrealCentimeters(MossVector).Equals(UnrealVector, 1.0e-6));

    const FQuat UnrealRotation = FRotator(17.0, -31.0, 42.0).Quaternion();
    const FQuat RoundTripRotation = FMossPhysicsUnits::ToUnrealRotation(
        FMossPhysicsUnits::ToMossRotation(UnrealRotation));
    TestTrue(TEXT("Quaternion basis conversion round-trips"), RoundTripRotation.Equals(UnrealRotation, 1.0e-5));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossPhysicsLayerSettingsAutomationTest,
    "MossUnreal.Physics.CollisionLayerDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossPhysicsLayerSettingsAutomationTest::RunTest(const FString& Parameters)
{
    const UMossPhysicsSettings* Settings = GetDefault<UMossPhysicsSettings>();
    TestNotNull(TEXT("Moss Physics project settings are available"), Settings);
    if (!Settings)
    {
        return false;
    }

    FString Error;
    TestTrue(TEXT("Default object and broadphase layers validate"), Settings->ValidateLayers(Error));
    TestEqual(TEXT("WorldStatic maps to object layer zero"), Settings->FindObjectLayer(TEXT("WorldStatic")), 0);
    TestEqual(TEXT("WorldDynamic maps to object layer one"), Settings->FindObjectLayer(TEXT("WorldDynamic")), 1);
    TestEqual(TEXT("Character maps to object layer two"), Settings->FindObjectLayer(TEXT("Character")), 2);
    TestEqual(TEXT("Vehicle maps to object layer three"), Settings->FindObjectLayer(TEXT("Vehicle")), 3);
    TestEqual(TEXT("Sensor maps to object layer four"), Settings->FindObjectLayer(TEXT("Sensor")), 4);
    TestEqual(TEXT("Unknown layer safely falls back to layer zero"), Settings->FindObjectLayer(TEXT("MissingLayer")), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossDataStoreJsonAutomationTest,
    "MossUnreal.Data.JsonRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossDataStoreJsonAutomationTest::RunTest(const FString& Parameters)
{
    FMossDataStore Store;
    Store.SetString(TEXT("name"), TEXT("Moss"));
    Store.SetNumber(TEXT("version"), 2.0);
    Store.SetBool(TEXT("enabled"), true);

    FString Json = TEXT("stale-prefix");
    TestTrue(TEXT("Data store serializes"), Store.ToJson(Json));
    TestFalse(TEXT("Serialization replaces existing output content"), Json.StartsWith(TEXT("stale-prefix")));

    FMossDataStore Loaded;
    FString Error;
    TestTrue(TEXT("Data store deserializes"), Loaded.FromJson(Json, Error));

    FString Name;
    double Version = 0.0;
    bool bEnabled = false;
    TestTrue(TEXT("String field is available"), Loaded.GetString(TEXT("name"), Name));
    TestEqual(TEXT("String value survives round trip"), Name, FString(TEXT("Moss")));
    TestTrue(TEXT("Number field is available"), Loaded.GetNumber(TEXT("version"), Version));
    TestEqual(TEXT("Number value survives round trip"), Version, 2.0);
    TestTrue(TEXT("Boolean field is available"), Loaded.GetBool(TEXT("enabled"), bEnabled));
    TestTrue(TEXT("Boolean value survives round trip"), bEnabled);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossUtilitiesIdentifierAutomationTest,
    "MossUnreal.Utilities.IdentifierValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossUtilitiesIdentifierAutomationTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Underscore-prefixed identifiers are valid"), FMossUtilities::IsValidIdentifier(TEXT("_moss_2")));
    TestTrue(TEXT("Alphabetic identifiers are valid"), FMossUtilities::IsValidIdentifier(TEXT("MossFramework")));
    TestFalse(TEXT("Identifiers cannot start with a digit"), FMossUtilities::IsValidIdentifier(TEXT("2Moss")));
    TestFalse(TEXT("Identifiers cannot contain spaces"), FMossUtilities::IsValidIdentifier(TEXT("Moss Framework")));
    TestFalse(TEXT("Empty identifiers are invalid"), FMossUtilities::IsValidIdentifier(FString()));
    TestEqual(TEXT("Non-finite delta times clamp to zero"), FMossUtilities::ClampDeltaSeconds(TNumericLimits<float>::QuietNaN()), 0.0f);
    return true;
}

class FMossTestSystem final : public IMossSystem
{
public:
    FName GetSystemName() const override { return TEXT("AutomationSystem"); }
    bool Initialize(const FMossSystemContext&, FString&) override { bWasInitialized = true; return true; }
    void Tick(const FMossSystemContext&) override { ++TickCount; }
    void Shutdown() override { bWasShutdown = true; }

    bool bWasInitialized = false;
    bool bWasShutdown = false;
    int32 TickCount = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossSystemRegistryAutomationTest,
    "MossUnreal.Systems.Lifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossSystemRegistryAutomationTest::RunTest(const FString& Parameters)
{
    FMossSystemRegistry Registry;
    TSharedRef<FMossTestSystem, ESPMode::ThreadSafe> System = MakeShared<FMossTestSystem, ESPMode::ThreadSafe>();
    FString Error;
    TestTrue(TEXT("System registration succeeds"), Registry.RegisterSystem(System, Error));
    TestTrue(TEXT("Registry initializes"), Registry.Initialize(Error));
    Registry.Tick(1.0 / 60.0);
    TestEqual(TEXT("System ticks once"), System->TickCount, 1);
    Registry.Shutdown();
    TestTrue(TEXT("System shutdown is called"), System->bWasShutdown);
    return true;
}

class FMossAutomationBackend final : public IMossBackend
{
public:
    explicit FMossAutomationBackend(const int32 InAbiVersion) : AbiVersion(InAbiVersion) {}

    bool Initialize(const FMossRuntimeConfig&, FString&) override { bInitialized = true; return true; }
    void Shutdown() override { bInitialized = false; }
    bool IsInitialized() const override { return bInitialized; }
    FMossBackendInfo GetInfo() const override
    {
        FMossBackendInfo Info;
        Info.Name = TEXT("AutomationBackend");
        Info.Version = TEXT("test");
        Info.AbiVersion = AbiVersion;
        return Info;
    }
    bool CreateWorld(const FMossNativeWorldDescription&, FMossNativeWorldHandle& OutWorld, FString&) override
    {
        OutWorld.Value = 1;
        return true;
    }
    void DestroyWorld(FMossNativeWorldHandle) override {}
    bool StepWorld(FMossNativeWorldHandle, double, FString&) override { return true; }
    bool CreateBody(FMossNativeWorldHandle, const FMossNativeBodyDescription&, FMossNativeBodyHandle& OutBody, FString&) override
    {
        OutBody.Value = 2;
        return true;
    }
    void DestroyBody(FMossNativeWorldHandle, FMossNativeBodyHandle) override {}
    bool SetBodyTransform(FMossNativeWorldHandle, FMossNativeBodyHandle, const FTransform&, FString&) override { return true; }
    bool GetBodyTransform(FMossNativeWorldHandle, FMossNativeBodyHandle, FTransform& OutTransform, FString&) const override
    {
        OutTransform = FTransform::Identity;
        return true;
    }
    bool Raycast(FMossNativeWorldHandle, const FMossNativeRaycastQuery&, FMossNativeRaycastHit&, FString&) const override
    {
        return true;
    }

private:
    int32 AbiVersion = 1;
    bool bInitialized = false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMossRuntimeBackendAutomationTest,
    "MossUnreal.Core.Runtime.BackendRegistrationAndAbi",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMossRuntimeBackendAutomationTest::RunTest(const FString& Parameters)
{
    FMossRuntime& Runtime = FMossRuntime::Get();
    if (Runtime.IsInitialized() || Runtime.HasBackendFactory())
    {
        AddInfo(TEXT("Skipping global runtime test because a real or external provider is already registered."));
        return true;
    }

    TestTrue(TEXT("Test provider factory registers"), Runtime.RegisterBackendFactory([]()
        -> TSharedPtr<IMossBackend, ESPMode::ThreadSafe>
    {
        return MakeShared<FMossAutomationBackend, ESPMode::ThreadSafe>(99);
    }, true));

    FString Error;
    FMossRuntimeConfig Config;
    TestFalse(TEXT("Mismatched provider ABI is rejected"), Runtime.Initialize(Config, Error));
    TestFalse(TEXT("Runtime remains uninitialized after ABI rejection"), Runtime.IsInitialized());
    TestTrue(TEXT("ABI mismatch explains the failure"), Error.Contains(TEXT("ABI mismatch")));

    TestTrue(TEXT("Valid provider factory replaces mismatched test provider"), Runtime.RegisterBackendFactory([]()
        -> TSharedPtr<IMossBackend, ESPMode::ThreadSafe>
    {
        return MakeShared<FMossAutomationBackend, ESPMode::ThreadSafe>(1);
    }, true));

    TestTrue(TEXT("Compatible provider initializes"), Runtime.Initialize(Config, Error));
    TestTrue(TEXT("Runtime reports ready"), Runtime.IsInitialized());
    TestEqual(TEXT("Backend metadata is exposed"), Runtime.GetBackendInfo().Name, FString(TEXT("AutomationBackend")));
    Runtime.Shutdown();
    TestFalse(TEXT("Runtime shuts down"), Runtime.IsInitialized());
    TestTrue(TEXT("Test provider factory is removed after the test"), Runtime.UnregisterBackendFactory());
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
