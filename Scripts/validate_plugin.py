#!/usr/bin/env python3
"""Source-level MossUnreal validation; this does not replace UnrealBuildTool."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PLUGIN = json.loads((ROOT / "MossUnreal.uplugin").read_text(encoding="utf-8"))
MODULES = {module["Name"]: module["Type"] for module in PLUGIN.get("Modules", [])}
assert PLUGIN.get("FileVersion") == 3
assert PLUGIN.get("VersionName") == "0.4.0"
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

native_header = (ROOT / "Source/MossUnreal/Public/Core/MossNative.h").read_text(encoding="utf-8")
assert "<Moss/Moss.h>" not in native_header, "Do not include the broad Moss umbrella in Unreal translation units"
for header in (
    "<Moss/Resources/Json.h>", "<Moss/Resources/Timer.h>", "<Moss/Moss_Platform.h>",
    "<Moss/Moss_Audio.h>", "<Moss/Moss_Network.h>", "<Moss/Variants/Vector/Vec3.h>",
):
    assert header in native_header, f"Native header boundary is missing {header}"

build_cs = (ROOT / "Source/MossUnreal/MossUnreal.Build.cs").read_text(encoding="utf-8")
for option in ("MOSS_INCLUDE_DIR", "MOSS_LIBRARY_FILE", "MOSS_EXTRA_LIBRARIES", "MOSS_ENABLE_GUI", "MOSS_ENABLE_XR", "MOSS_GRAPHICS_VULKAN", "MOSS_GRAPHICS_DIRECTX", "MOSS_GRAPHICS_OPENGL", "MOSS_GRAPHICS_OPENGLES", "MOSS_GRAPHICS_METAL"):
    assert option in build_cs, f"Build.cs does not configure {option}"

resources_header = (ROOT / "Source/MossUnreal/Public/Resources/Resources.h").read_text(encoding="utf-8")
assert "ValidateAndSerializeJson" in resources_header and "SerializedJson" in resources_header
assert "CanonicalJson" not in resources_header, "Resource serialization must not claim canonical member ordering"

native_audit = (ROOT / "Docs/NATIVE_HEADER_AUDIT.md").read_text(encoding="utf-8")
for blocker in ("Resources/Config.h", "Resources/Tween.h", "Resources/Gradient.h", "Resources/BezierCurve.h"):
    assert blocker in native_audit, f"Native header audit does not record {blocker} limitations"

native_class_map = (ROOT / "Docs/NativeClassMap.md").read_text(encoding="utf-8")
for header in ("Moss_Platform.h", "Moss_Audio.h", "Moss_Network.h", "Moss_GUI.h", "Moss_XR.h", "Resources/Timer.h"):
    assert header in native_class_map, f"Native class map does not document {header}"

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
