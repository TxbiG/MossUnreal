#!/usr/bin/env python3
"""Source-level MossUnreal validation; this does not replace UnrealBuildTool."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PLUGIN = json.loads((ROOT / "MossUnreal.uplugin").read_text(encoding="utf-8"))
MODULES = {module["Name"]: module["Type"] for module in PLUGIN.get("Modules", [])}
assert PLUGIN.get("FileVersion") == 3
assert PLUGIN.get("VersionName") == "0.5.2"
assert MODULES.get("MossUnreal") == "Runtime"
assert MODULES.get("MossUnrealTests") == "Editor"

required = [
    "Source/MossUnreal/MossUnreal.Build.cs",
    "Source/MossUnreal/Public/Core/Core.h",
    "Source/MossUnreal/Public/Core/MossBackend.h",
    "Source/MossUnreal/Public/Core/MossNative.h",
    "Source/MossUnreal/Public/Core/MossRuntime.h",
    "Source/MossUnreal/Private/Core/MossRuntime.cpp",
    "Source/MossUnreal/Public/Core/MossRuntimeSubsystem.h",
    "Source/MossUnreal/Private/Core/MossRuntimeSubsystem.cpp",
    "Source/MossUnreal/Public/Math/MossMath.h",
    "Source/MossUnreal/Private/Math/MossMath.cpp",
    "Source/MossUnreal/Public/Math/MossNativeMath.h",
    "Source/MossUnreal/Private/Math/MossNativeMath.cpp",
    "Source/MossUnreal/Public/Utilities/MossNativeTimer.h",
    "Source/MossUnreal/Private/Utilities/MossNativeTimer.cpp",
    "Source/MossUnreal/Public/Utilities/MossUtilities.h",
    "Source/MossUnreal/Private/Utilities/MossUtilities.cpp",
    "Source/MossUnreal/Public/Diagnostics/MossDiagnostics.h",
    "Source/MossUnreal/Private/Diagnostics/MossDiagnostics.cpp",
    "Source/MossUnreal/Public/Data/MossDataStore.h",
    "Source/MossUnreal/Private/Data/MossDataStore.cpp",
    "Source/MossUnreal/Public/Systems/MossSystem.h",
    "Source/MossUnreal/Private/Systems/MossSystem.cpp",
    "Source/MossUnreal/Public/Application/Platform/Platform.h",
    "Source/MossUnreal/Private/Application/Platform/Platform.cpp",
    "Source/MossUnreal/Public/Application/Input/Input.h",
    "Source/MossUnreal/Private/Application/Input/Input.cpp",
    "Source/MossUnreal/Public/Audio/AudioSystem.h",
    "Source/MossUnreal/Private/Audio/AudioSystem.cpp",
    "Source/MossUnreal/Public/Networking/Network.h",
    "Source/MossUnreal/Private/Networking/Network.cpp",
    "Source/MossUnreal/Public/Resources/Resources.h",
    "Source/MossUnreal/Private/Resources/Resources.cpp",
    "Source/MossUnreal/Public/GUI/GUI.h",
    "Source/MossUnreal/Private/GUI/GUI.cpp",
    "Source/MossUnreal/Public/XR/XRSystem.h",
    "Source/MossUnreal/Private/XR/XRSystem.cpp",
    "Source/MossUnreal/Public/XR/XRController.h",
    "Source/MossUnreal/Private/XR/XRController.cpp",
    "Source/MossUnreal/Public/Navigation/NavigationMesh.h",
    "Source/MossUnreal/Private/Navigation/NavigationMesh.cpp",
    "Source/MossUnreal/Public/Physics/MossCharacterComponent.h",
    "Source/MossUnreal/Private/Physics/MossCharacterComponent.cpp",
    "Source/MossUnreal/Public/Physics/MossSoftBodyComponent.h",
    "Source/MossUnreal/Private/Physics/MossSoftBodyComponent.cpp",
    "Source/MossUnreal/Public/Physics/MossVehicleComponent.h",
    "Source/MossUnreal/Private/Physics/MossVehicleComponent.cpp",
    "Source/MossUnreal/Public/Physics/MossWheelComponent.h",
    "Source/MossUnreal/Private/Physics/MossWheelComponent.cpp",
    "Source/MossUnreal/Public/Physics/MossPhysicsWorldSubsystem.h",
    "Source/MossUnreal/Private/MossPhysicsWorldSubsystem.cpp",
    "Source/MossUnreal/Public/Physics/MossPhysicsBodyComponent.h",
    "Source/MossUnreal/Private/MossPhysicsBodyComponent.cpp",
    "Source/MossUnreal/Public/Blueprint/MossBlueprintLibrary.h",
    "Source/MossUnreal/Private/Blueprint/MossBlueprintLibrary.cpp",
    "Source/MossUnrealTests/MossUnrealTests.Build.cs",
    "Source/MossUnrealTests/Private/MossUnrealAutomationTests.cpp",
    "Docs/NATIVE_HEADER_AUDIT.md",
    "Docs/NativeClassMap.md",
    "Docs/NativeAPI.md",
    "Examples/NativeCpp/DirectMossApiExample.cpp",
    "Examples/UnrealCpp/MossPhysicsCrate/MossPhysicsCrateActor.h",
    "Examples/UnrealCpp/MossPhysicsCrate/MossPhysicsCrateActor.cpp",
    "Examples/UnrealCpp/MossPhysicsCrate/README.md",
    "Examples/UnrealBlueprint/MossActorTagWorkflow.md",
    "Docs/MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md",
    "Scripts/validate_moss_headers.py",
]
for item in required:
    path = ROOT / item
    assert path.is_file() and path.stat().st_size > 0, f"Missing or empty required source file: {item}"

backend = (ROOT / "Source/MossUnreal/Public/Core/MossBackend.h").read_text(encoding="utf-8")
for symbol in ("Initialize(", "CreateWorld(", "DestroyWorld(", "StepWorld(", "CreateBody(", "DestroyBody(", "SetBodyTransform(", "GetBodyTransform(", "Raycast("):
    assert symbol in backend, f"Backend contract is missing {symbol}"

runtime_subsystem = (ROOT / "Source/MossUnreal/Private/Core/MossRuntimeSubsystem.cpp").read_text(encoding="utf-8")
assert "TryInitializeRuntime" in runtime_subsystem, "Late provider registration retry is missing"
runtime_header = (ROOT / "Source/MossUnreal/Public/Core/MossRuntime.h").read_text(encoding="utf-8")
assert "UnregisterBackendFactory" in runtime_header, "Controlled provider factory cleanup is missing"
physics_world = (ROOT / "Source/MossUnreal/Private/MossPhysicsWorldSubsystem.cpp").read_text(encoding="utf-8")
assert "TryCreatePendingBodies" in physics_world, "Queued body creation retry is missing"
assert "FMossPhysicsScene" in physics_world and "IMossBackend" not in physics_world, "Physics world must own the direct Moss scene, not a provider abstraction"
scene_header = (ROOT / "Source/MossUnreal/Private/Physics/MossPhysicsScene.h").read_text(encoding="utf-8")
build_cs = (ROOT / "Source/MossUnreal/MossUnreal.Build.cs").read_text(encoding="utf-8")
for symbol in ("PhysicsSystem", "BodyInterface", "JobSystemThreadPool", "TempAllocatorImplWithMallocFallback"):
    assert symbol in (ROOT / "Source/MossUnreal/Private/Physics/MossPhysicsScene.cpp").read_text(encoding="utf-8"), f"Direct physics adapter is missing {symbol}"
body_component = (ROOT / "Source/MossUnreal/Private/MossPhysicsBodyComponent.cpp").read_text(encoding="utf-8")
for symbol in ("UBoxComponent", "USphereComponent", "UCapsuleComponent", "FKConvexElem", "FKBoxElem", "FKSphereElem", "FKSphylElem"):
    assert symbol in body_component, f"Compound body extraction is missing {symbol}"
for symbol in ("AddForce", "AddImpulse", "AddTorque", "SetLinearVelocity", "GetLinearVelocity", "SetAngularVelocity", "GetAngularVelocity"):
    assert symbol in body_component, f"Body component is missing {symbol}"
scene_source = (ROOT / "Source/MossUnreal/Private/Physics/MossPhysicsScene.cpp").read_text(encoding="utf-8")
for symbol in ("PhysicsSystem", "BodyInterface", "BodyCreationSettings", "StaticCompoundShapeSettings", "JobSystemThreadPool", "TempAllocatorImplWithMallocFallback", "ContactListener", "AddImpulse", "AddTorque", "GetLinearVelocity", "GetAngularVelocity"):
    assert symbol in scene_source, f"Direct Moss physics scene is missing {symbol}"
for symbol in ("MOSSUNREAL_ENABLE_MOSS_PHYSICS", "MOSS_PHYSICS_HEADERS_READY"):
    assert symbol in build_cs, f"Physics safety gate is missing {symbol}"
example_actor = (ROOT / "Examples/UnrealCpp/MossPhysicsCrate/MossPhysicsCrateActor.cpp").read_text(encoding="utf-8")
assert "SetRootComponent(MossBody)" in example_actor and "ECollisionEnabled::QueryOnly" in example_actor

native_header = (ROOT / "Source/MossUnreal/Public/Core/MossNative.h").read_text(encoding="utf-8")
assert "<Moss/Moss.h>" not in native_header, "Do not include the broad Moss umbrella in Unreal translation units"
for header in (
    "<Moss/Resources/Json.h>", "<Moss/Resources/Timer.h>", "<Moss/Moss_Platform.h>",
    "<Moss/Moss_Audio.h>", "<Moss/Moss_Network.h>", "<Moss/Variants/Vector/Vec3.h>",
):
    assert header in native_header, f"Native header boundary is missing {header}"

for option in ("MOSS_INCLUDE_DIR", "MOSS_LIBRARY_FILE", "MOSS_EXTRA_LIBRARIES", "MOSS_ENABLE_GUI", "MOSS_ENABLE_XR", "MOSS_ENABLE_PHYSICS", "MOSS_PHYSICS_HEADERS_READY", "MOSS_GRAPHICS_VULKAN", "MOSS_GRAPHICS_DIRECTX", "MOSS_GRAPHICS_OPENGL", "MOSS_GRAPHICS_OPENGLES", "MOSS_GRAPHICS_METAL"):
    assert option in build_cs, f"Build.cs does not configure {option}"

resources_header = (ROOT / "Source/MossUnreal/Public/Resources/Resources.h").read_text(encoding="utf-8")
assert "ValidateAndSerializeJson" in resources_header and "SerializedJson" in resources_header
assert "CanonicalJson" not in resources_header, "Resource serialization must not claim canonical member ordering"

integration_doc = (ROOT / "Docs/MOSS_PHYSICS_UNREALJOLT_INTEGRATION.md").read_text(encoding="utf-8")
for topic in ("PhysicsSystem::Init", "BodyInterface::CreateAndAddBody", "AddImpulse", "AddTorque", "Collision layers and matrix", "Debug drawing", "compile-gated"):
    assert topic in integration_doc, f"Direct physics integration documentation is missing {topic}"

native_audit = (ROOT / "Docs/NATIVE_HEADER_AUDIT.md").read_text(encoding="utf-8")
for blocker in ("Resources/Config.h", "Resources/Tween.h", "Resources/Gradient.h", "Resources/BezierCurve.h"):
    assert blocker in native_audit, f"Native header audit does not record {blocker} limitations"

native_class_map = (ROOT / "Docs/NativeClassMap.md").read_text(encoding="utf-8")
for header in ("Moss_Platform.h", "Moss_Audio.h", "Moss_Network.h", "Moss_GUI.h", "Moss_XR.h", "Resources/Timer.h"):
    assert header in native_class_map, f"Native class map does not document {header}"

# UnrealJolt-style opt-in actor tags and strict single-owner Chaos policy.
settings_header = (ROOT / "Source/MossUnreal/Public/Physics/MossPhysicsSettings.h").read_text(encoding="utf-8")
for symbol in ("bEnableActorTagDiscovery", "StaticActorTag", "DynamicActorTag", "StaticActorCollisionLayer", "DynamicActorCollisionLayer"):
    assert symbol in settings_header, f"Physics settings are missing actor-tag setting {symbol}"
for tag in ("moss-static", "moss-dynamic"):
    assert tag in settings_header, f"Default Moss actor tag is missing: {tag}"
for symbol in ("AddOnActorSpawnedHandler", "RefreshTaggedActors", "RegisterActorFromTags", "SetRootComponent", "ActorHasTag", "EComponentMobility::Static"):
    assert symbol in physics_world, f"Actor-tag discovery is missing {symbol}"
body_header = (ROOT / "Source/MossUnreal/Public/Physics/MossPhysicsBodyComponent.h").read_text(encoding="utf-8")
for symbol in ("EMossChaosConflictPolicy", "DisableChaosPhysicsCollision", "RejectMossBody", "ResolveChaosOwnership"):
    assert symbol in body_header, f"Chaos ownership policy is missing {symbol}"
assert "Primitive->SetSimulatePhysics(false)" in body_component, "Moss ownership must explicitly disable Chaos simulation when selected"
assert "ECollisionEnabled::QueryOnly" in body_component and "ECollisionEnabled::NoCollision" in body_component, "Moss-owned primitives must be excluded from Chaos physical collision"
assert "RestoreChaosOwnership" in body_component and "SetCollisionEnabled(State.PreviousCollisionEnabled)" in body_component, "Original Chaos primitive state must be restored on component teardown"
assert "PhysicsWorld->UnregisterBody(this)" in body_component, "Rejected re-creation must not leave a previous Moss body active"
assert "PreviousChaosPrimitiveStates" in body_header and "RestoreChaosOwnership" in body_component, "Chaos state must be retained and restored across Moss ownership"
assert "return true;" in physics_world[physics_world.index("bool UMossPhysicsWorldSubsystem::RegisterBody"):physics_world.index("bool UMossPhysicsWorldSubsystem::CreateBodyForComponent")], "Transient scene initialization failures must retain pending registrations"
assert "Component->RestoreChaosOwnership()" in physics_world, "Fatal pending body creation/world teardown must restore original Chaos state"
assert "for (int32 Index = RegisteredBodies.Num() - 1; Index >= 0; --Index)" in physics_world, "Pending-body retry must not mutate RegisteredBodies during a range-for loop"
assert "PhysicsWorld->UnregisterBody(this);\n    RestoreChaosOwnership();" in body_component, "Re-creation must retire the old Moss body before restoring prior Chaos state"
assert physics_world.count("World->IsGameWorld()") >= 3, "Actor discovery/refresh/ticking must be restricted to game worlds"
assert "ActivateBody(Id)" in scene_source, "WakeBody must use Moss BodyInterface::ActivateBody"
assert "PhysicsWorld->WakeBody(NativeBody, Error)" in body_component, "WakeBody must be exposed through the Unreal body component"


# UnrealJolt-style body properties must be reachable through the component,
# world subsystem, and direct Moss Physics scene, not only stored as editor data.
runtime_body_setters = (
    "SetMass", "SetLinearDamping", "SetAngularDamping", "SetMaxLinearVelocity",
    "SetMaxAngularVelocity", "SetAllowSleeping", "SetMotionQuality",
    "SetEnhancedInternalEdgeRemoval", "SetApplyGyroscopicForce", "SetSolverSteps",
    "SetSensor", "SetObjectLayer",
)
world_header = (ROOT / "Source/MossUnreal/Public/Physics/MossPhysicsWorldSubsystem.h").read_text(encoding="utf-8")
scene_header = (ROOT / "Source/MossUnreal/Private/Physics/MossPhysicsScene.h").read_text(encoding="utf-8")
for symbol in runtime_body_setters:
    component_symbol = "SetCollisionLayer" if symbol == "SetObjectLayer" else symbol
    assert component_symbol in body_header and component_symbol in body_component, f"Blueprint/C++ body component is missing runtime setter {component_symbol}"
    assert symbol in world_header and symbol in physics_world, f"World subsystem is missing runtime setter {symbol}"
    assert symbol in scene_header and symbol in scene_source, f"Direct Moss Physics scene is missing runtime setter {symbol}"
assert "BodyMotionTypes.find(BodyId)" in scene_source, "Motion-quality updates must not create stale body-map entries"
assert "ObjectLayerPairs->Layers.begin()" in scene_source, "Runtime object-layer changes must reject unconfigured layer indices"
settings_source = (ROOT / "Source/MossUnreal/Private/Physics/MossPhysicsSettings.cpp").read_text(encoding="utf-8")
assert "StaticActorCollisionLayer" in settings_source and "DynamicActorCollisionLayer" in settings_source, "Actor-tag object layers must be validated before scene initialization"
assert "StaticActorTag == DynamicActorTag" in settings_source, "Static and dynamic actor tags must be distinct"

# UHT convention: generated.h must be the final include in reflected headers.
for header_path in (ROOT / "Source/MossUnreal/Public").rglob("*.h"):
    lines = [line.strip() for line in header_path.read_text(encoding="utf-8").splitlines() if line.strip().startswith("#include")]
    generated = [i for i, line in enumerate(lines) if ".generated.h" in line]
    if generated:
        assert generated[-1] == len(lines) - 1, f"Generated header is not the last include: {header_path.relative_to(ROOT)}"

bad_nested_source = ROOT / "Source/MossUnreal/Source"
assert not bad_nested_source.exists(), "Unexpected nested Source/MossUnreal/Source directory"

print(f"MossUnreal {PLUGIN.get('VersionName')} source validation passed ({len(required)} required files).")
print("Note: UnrealBuildTool/UHT and final native linking require their target build environments.")
