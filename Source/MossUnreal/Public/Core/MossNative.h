#pragma once

/**
 * Opt-in include boundary for the public Moss modules used by this plugin.
 *
 * Do not include this from a reflected Unreal header. Moss's headers declare
 * a global TArray used by its Float3/Float4 helper aliases. Unreal also owns a
 * global TArray template. The temporary token rename below isolates Moss's
 * private helper alias while leaving Vec/Quat/API types and their signatures
 * unchanged. Avoid including Moss/Moss.h here: it additionally exposes Moss's
 * global TMap/TSet and other container declarations which conflict with UE.
 *
 * GUI and XR are separate opt-ins because they include Moss_Renderer and
 * its selected renderer declarations. Vulkan headers are required only when
 * the Moss build enables MOSS_GRAPHICS_VULKAN. Physics and Recast navigation
 * remain disabled for this revision because the uploaded public header dependency chains fail
 * standalone compilation; see Docs/NATIVE_HEADER_AUDIT.md.
 */
#if defined(MOSSUNREAL_HAS_MOSS_API) && MOSSUNREAL_HAS_MOSS_API
    #if defined(__linux__) || defined(__APPLE__)
        #include <sys/stat.h>
        #ifndef MOSS_NO_FORCE_INLINE
            #define MOSS_NO_FORCE_INLINE 1
        #endif
    #endif

    #ifndef TArray
        #define TArray MossFrameworkTArray
        #define MOSSUNREAL_UNDEFINE_TEMP_TARRAY 1
    #endif

    #include <Moss/Moss_stdinc.h>
    #include <Moss/Variants/Vector/Float2.h>
    #include <Moss/Variants/Vector/Float3.h>
    #include <Moss/Variants/Vector/Float4.h>
    #include <Moss/Variants/Vector/Vec4.h>
    #include <Moss/Variants/Vector/UVec4.h>
    #include <Moss/Variants/Vector/Vec2.h>
    #include <Moss/Variants/Vector/Vec3.h>
    #include <Moss/Variants/Quat.h>
    #include <Moss/Variants/Matrix/Mat44.h>
    #include <Moss/Resources/Json.h>
    #include <Moss/Resources/Timer.h>
    #include <Moss/Moss_Platform.h>
    #include <Moss/Moss_Audio.h>
    #include <Moss/Moss_Network.h>

    #ifdef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
        #undef TArray
        #undef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
    #endif

    #define MOSSUNREAL_NATIVE_API_AVAILABLE 1
#else
    #define MOSSUNREAL_NATIVE_API_AVAILABLE 0
#endif

#if MOSSUNREAL_NATIVE_API_AVAILABLE && defined(MOSSUNREAL_ENABLE_MOSS_GUI) && MOSSUNREAL_ENABLE_MOSS_GUI
    #ifndef TArray
        #define TArray MossFrameworkTArray
        #define MOSSUNREAL_UNDEFINE_TEMP_TARRAY 1
    #endif
    #include <Moss/Moss_GUI.h>
    #ifdef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
        #undef TArray
        #undef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
    #endif
    #define MOSSUNREAL_GUI_API_AVAILABLE 1
#else
    #define MOSSUNREAL_GUI_API_AVAILABLE 0
#endif

#if MOSSUNREAL_NATIVE_API_AVAILABLE && defined(MOSSUNREAL_ENABLE_MOSS_XR) && MOSSUNREAL_ENABLE_MOSS_XR
    #ifndef TArray
        #define TArray MossFrameworkTArray
        #define MOSSUNREAL_UNDEFINE_TEMP_TARRAY 1
    #endif
    #include <Moss/Moss_XR.h>
    #ifdef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
        #undef TArray
        #undef MOSSUNREAL_UNDEFINE_TEMP_TARRAY
    #endif
    #define MOSSUNREAL_XR_API_AVAILABLE 1
#else
    #define MOSSUNREAL_XR_API_AVAILABLE 0
#endif

// Physics is included privately by MossPhysicsScene.cpp only. Do not include
// the full physics dependency graph from this general-purpose umbrella header.
#if defined(MOSSUNREAL_ENABLE_MOSS_NAVIGATION) && MOSSUNREAL_ENABLE_MOSS_NAVIGATION
    #error "Moss navigation headers in this framework revision are not compile-ready; see Docs/NATIVE_HEADER_AUDIT.md."
#endif
