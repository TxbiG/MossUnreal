#!/usr/bin/env python3
"""Compile-check MossUnreal's selected Moss headers against a Moss checkout.

Requires a C++17 compiler with -fsyntax-only support (GCC or Clang) and
MOSS_INCLUDE_DIR pointing at the public include root containing Moss/.
Optional GUI/XR checks are enabled when MOSS_ENABLE_GUI / MOSS_ENABLE_XR are set.
"""
from __future__ import annotations

import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE_DIR = os.environ.get("MOSS_INCLUDE_DIR")
if not INCLUDE_DIR:
    sys.exit("Set MOSS_INCLUDE_DIR to the Moss public include root (the folder containing Moss/).")
INCLUDE = Path(INCLUDE_DIR).resolve()
if not (INCLUDE / "Moss").is_dir():
    sys.exit(f"MOSS_INCLUDE_DIR does not contain Moss/: {INCLUDE}")

compiler_setting = os.environ.get("CXX", "c++")
compiler_parts = shlex.split(compiler_setting)
compiler = shutil.which(compiler_parts[0])
if not compiler:
    sys.exit(f"C++ compiler not found: {compiler_parts[0]}")
compiler_parts[0] = compiler

source = r'''template <class T> class TArray; // simulate UE's global container declaration
#define MOSSUNREAL_HAS_MOSS_API 1
#include "Core/MossNative.h"
int main()
{
    // Native math, timing and JSON declarations used by MossUnreal wrappers.
    Vec3 A(1.0f, 2.0f, 3.0f), B(4.0f, 5.0f, 6.0f);
    Vec3 C = A.Cross(B).NormalizedOr(Vec3::Zero());
    (void)A.Dot(B); (void)A.Length(); (void)Vec3::Min(A, B); (void)Vec3::Max(A, B);
    Quat Q(0.0f, 0.0f, 0.0f, 1.0f);
    Quat R = Q.SLERP(Q, 0.5f).Normalized();
    Timer T(1.0f, false); T.setCallback([]() {}); T.update(0.01f); T.pause(); T.resume();
    auto Document = Json::parse("{}");
    if (Document) (void)Document->stringify();
    (void)C; (void)R;

    // Platform, keyboard/mouse and gamepad declarations.
    (void)Moss_GetAvailableCPUCores(); (void)Moss_GetCPUCacheLineSize(); (void)Moss_GetSystemRAM();
    (void)Moss_OpenURL("https://example.invalid");
    (void)Moss_CopyFile("a", "b", false); (void)Moss_CreateDirectory("x", true);
    (void)Moss_RemovePath("x", false); (void)Moss_RenamePath("a", "b", false);
    Moss_PathInfo PathInfo{}; (void)Moss_GetPathInfo("x", &PathInfo);
    char Path[256]{}; (void)Moss_GetCurrentDirectory(Path, 256); (void)Moss_GetBasePath(Path, 256);
    (void)Moss_GetPrefPath("org", "app", Path, 256);
    (void)Moss_IsKeyPressed(Keyboard::KEY_A); (void)Moss_IsReleased(Keyboard::KEY_A);
    (void)Moss_IsKeyJustPressed(Keyboard::KEY_A); (void)Moss_IsKeyJustReleased(Keyboard::KEY_A);
    (void)Moss_IsMousePressed(Mouse::LEFT); (void)Moss_IsMouseJustPressed(Mouse::LEFT);
    int MouseX = 0, MouseY = 0; Moss_GetMousePosition(&MouseX, &MouseY); Moss_SetMousePosition(MouseX, MouseY);
    Moss_SetMouseVisible(true);
    Moss_Gamepad* Gamepad = Moss_OpenGamepad(0);
    if (Gamepad) {
        (void)Moss_GamepadConnected(Gamepad); (void)Moss_GetGamepadName(Gamepad);
        (void)Moss_IsGamepadButtonPressed(Gamepad, Moss_GamepadButton::SOUTH);
        (void)Moss_GetGamepadAxis(Gamepad, GamepadAxis::LEFT_X);
        Moss_CloseGamepad(Gamepad);
    }
    (void)Moss_GetNumGamepads(); Moss_UpdateGamepads();

    // Audio source/stream and acoustic trace declarations.
    Moss_AudioSource* AudioSource = Moss_AudioLoadWavFile("sample.wav");
    if (AudioSource) Moss_AudioSourceDestroy(AudioSource);
    AudioStream2D* Stream2D = Moss_AudioStream2DCreate();
    if (Stream2D) { Moss_AudioStream2DSetVolume(Stream2D, 1.0f); Moss_AudioStream2DSetPitch(Stream2D, 1.0f); Moss_AudioStream2DSetLoop(Stream2D, false); Moss_AudioStream2DRemove(Stream2D); }
    AudioStream3D* Stream3D = Moss_AudioStream3DCreate();
    if (Stream3D) { Moss_AudioStream3DSetVolume(Stream3D, 1.0f); Moss_AudioStream3DSetPitch(Stream3D, 1.0f); Moss_AudioStream3DSetLoop(Stream3D, false); Moss_AudioStream3DRemove(Stream3D); }
    Moss_AudioRayTrace2DDesc Trace2D{}; Moss_AudioRayTrace3DDesc Trace3D{};
    Moss_AudioRayTraceResult TraceResult{};
    (void)Moss_AudioRayTrace2D(&Trace2D, &TraceResult); (void)Moss_AudioRayTrace3D(&Trace3D, &TraceResult);

    // ENet-style host, peer, packet and event declarations.
    (void)Moss_Init_Network();
    ENetAddress Address{}; enet_address_build_any(&Address, ENetAddressType::IPV4);
    ENetHost* Host = enet_host_create(ENetAddressType::IPV4, &Address, 8, 2, 0, 0);
    if (Host) {
        ENetPeer* Peer = enet_host_connect(Host, &Address, 2, 0);
        ENetPacket* Packet = enet_packet_create("x", 1, static_cast<uint32_t>(ENetPacketFlag::RELIABLE));
        if (Peer && Packet) (void)enet_peer_send(Peer, 0, Packet);
        else if (Packet) enet_packet_destroy(Packet);
        ENetEvent Event{}; (void)enet_host_service(Host, &Event, 0); enet_host_flush(Host); enet_host_destroy(Host);
    }
    Moss_TerminateNetwork();

#if MOSSUNREAL_GUI_API_AVAILABLE
    bool Open = true; float FloatValue = 0.5f; int IntValue = 1;
    (void)::BeginWindow("smoke", &Open, 0); ::Text("%s", "text"); ::RichText("text");
    (void)::DragFloat("float", &FloatValue); (void)::DragInt("int", &IntValue);
    ::SliderFloat(0.0f, 1.0f, FloatValue); ::SliderInt(0, 10, IntValue); ::EndWindow();
#endif
#if MOSSUNREAL_XR_API_AVAILABLE
    Moss_Renderer* Renderer = nullptr;
    (void)Moss_XR_Initialize(Renderer);
    MossXR_Session* Session = Moss_XR_CreateSession();
    if (Session) { (void)Moss_XR_BeginFrame(Session); Moss_XR_EndFrame(Session); Moss_XR_DestroySession(Session); }
    MossXR_ActionSet* ActionSet = Moss_XR_CreateActionSet("smoke");
    if (ActionSet) {
        MossXR_Action* Action = Moss_XR_CreateAction(ActionSet, "pose", MossXR_ActionType::Pose);
        if (Action) { MossXR_Pose Pose{}; (void)Moss_XR_GetActionPose(Action, &Pose); Moss_XR_DestroyAction(Action); }
        Moss_XR_DestroyActionSet(ActionSet);
    }
    Moss_XR_Shutdown();
#endif
    return 0;
}
'''

with tempfile.TemporaryDirectory(prefix="mossunreal-header-check-") as temp:
    source_path = Path(temp) / "mossunreal_header_check.cpp"
    source_path.write_text(source, encoding="utf-8")
    args = compiler_parts + ["-std=c++17", "-fsyntax-only", f"-I{INCLUDE}", f"-I{ROOT / 'Source/MossUnreal/Public'}"]
    gui_enabled = os.environ.get("MOSS_ENABLE_GUI", "").lower() in ("1", "true", "yes")
    xr_enabled = os.environ.get("MOSS_ENABLE_XR", "").lower() in ("1", "true", "yes")
    args.append("-DMOSSUNREAL_ENABLE_MOSS_GUI=1" if gui_enabled else "-DMOSSUNREAL_ENABLE_MOSS_GUI=0")
    args.append("-DMOSSUNREAL_ENABLE_MOSS_XR=1" if xr_enabled else "-DMOSSUNREAL_ENABLE_MOSS_XR=0")
    vulkan_include = os.environ.get("MOSS_VULKAN_INCLUDE_DIR")
    vulkan_sdk = os.environ.get("VULKAN_SDK")
    if vulkan_include:
        args.append(f"-I{Path(vulkan_include).resolve()}")
    elif vulkan_sdk:
        args.append(f"-I{(Path(vulkan_sdk) / 'Include').resolve()}")
    if os.environ.get("MOSS_GRAPHICS_VULKAN", "").lower() in ("1", "true", "yes"):
        args.append("-DMOSS_GRAPHICS_VULKAN=1")
    args.append(str(source_path))
    result = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.stdout:
        print(result.stdout, end="")
    if result.returncode:
        sys.exit(result.returncode)

print("MossUnreal selected Moss public headers and representative calls passed the C++17 syntax-only smoke test.")
print("This test does not perform library linking or UnrealBuildTool/UHT compilation.")
