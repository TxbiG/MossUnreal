using UnrealBuildTool;
using System;
using System.IO;

public class MossUnreal : ModuleRules
{
    public MossUnreal(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Json"
        });

        PrivateDependencyModuleNames.AddRange(new[] { "Projects" });

        bool bHasMossHeaders = false;
        bool bHasMossLibrary = false;
        bool bEnableMossGui = ReadBoolEnvironmentVariable("MOSS_ENABLE_GUI");
        bool bEnableMossXr = ReadBoolEnvironmentVariable("MOSS_ENABLE_XR");
        bool bMossGraphicsVulkan = ReadBoolEnvironmentVariable("MOSS_GRAPHICS_VULKAN");
        bool bMossGraphicsDirectX = ReadBoolEnvironmentVariable("MOSS_GRAPHICS_DIRECTX");
        bool bMossGraphicsOpenGL = ReadBoolEnvironmentVariable("MOSS_GRAPHICS_OPENGL");
        bool bMossGraphicsOpenGLES = ReadBoolEnvironmentVariable("MOSS_GRAPHICS_OPENGLES");
        bool bMossGraphicsMetal = ReadBoolEnvironmentVariable("MOSS_GRAPHICS_METAL");

        PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_HEADERS=0");
        PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_LIBRARY=0");
        PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_API=0");
        PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_GUI=0");
        PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_XR=0");
        PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_PHYSICS=0");
        PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_NAVIGATION=0");

        // Match the renderer backend macros used to build the Moss library.
        // These are only defined when explicitly enabled because Moss headers
        // use #ifdef tests for renderer selection.
        if (bMossGraphicsVulkan) PublicDefinitions.Add("MOSS_GRAPHICS_VULKAN=1");
        if (bMossGraphicsDirectX) PublicDefinitions.Add("MOSS_GRAPHICS_DIRECTX=1");
        if (bMossGraphicsOpenGL) PublicDefinitions.Add("MOSS_GRAPHICS_OPENGL=1");
        if (bMossGraphicsOpenGLES) PublicDefinitions.Add("MOSS_GRAPHICS_OPENGLES=1");
        if (bMossGraphicsMetal) PublicDefinitions.Add("MOSS_GRAPHICS_METAL=1");

        string MossIncludeDir = Environment.GetEnvironmentVariable("MOSS_INCLUDE_DIR");
        if (!String.IsNullOrWhiteSpace(MossIncludeDir))
        {
            MossIncludeDir = Path.GetFullPath(MossIncludeDir);
            if (!Directory.Exists(MossIncludeDir) || !Directory.Exists(Path.Combine(MossIncludeDir, "Moss")))
                throw new BuildException("MOSS_INCLUDE_DIR must be the public include root containing the Moss/ directory.");

            PublicSystemIncludePaths.Add(MossIncludeDir);
            PublicDefinitions.Remove("MOSSUNREAL_HAS_MOSS_HEADERS=0");
            PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_HEADERS=1");
            bHasMossHeaders = true;
        }

        string MossLibraryFile = Environment.GetEnvironmentVariable("MOSS_LIBRARY_FILE");
        if (!String.IsNullOrWhiteSpace(MossLibraryFile))
        {
            MossLibraryFile = Path.GetFullPath(MossLibraryFile);
            if (!File.Exists(MossLibraryFile))
                throw new BuildException("MOSS_LIBRARY_FILE does not exist: " + MossLibraryFile);

            PublicAdditionalLibraries.Add(MossLibraryFile);
            PublicDefinitions.Remove("MOSSUNREAL_HAS_MOSS_LIBRARY=0");
            PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_LIBRARY=1");
            bHasMossLibrary = true;
        }

        if (bHasMossLibrary && !bHasMossHeaders)
            throw new BuildException("MOSS_LIBRARY_FILE was set without MOSS_INCLUDE_DIR.");

        if (bHasMossHeaders && bHasMossLibrary)
        {
            PublicDefinitions.Remove("MOSSUNREAL_HAS_MOSS_API=0");
            PublicDefinitions.Add("MOSSUNREAL_HAS_MOSS_API=1");
        }
        else if (bHasMossHeaders != bHasMossLibrary)
        {
            System.Console.WriteLine("MossUnreal: Moss headers and library must both be configured before native calls are enabled.");
        }

        // The Moss platform/renderer headers include Vulkan's public header
        // only when the Vulkan renderer macro is enabled. Keep this selection
        // aligned with the library's own CMake renderer configuration.
        string VulkanIncludeDir = Environment.GetEnvironmentVariable("MOSS_VULKAN_INCLUDE_DIR");
        string VulkanSdk = Environment.GetEnvironmentVariable("VULKAN_SDK");
        if (String.IsNullOrWhiteSpace(VulkanIncludeDir) && !String.IsNullOrWhiteSpace(VulkanSdk))
            VulkanIncludeDir = Path.Combine(VulkanSdk, "Include");
        if (!String.IsNullOrWhiteSpace(VulkanIncludeDir))
        {
            VulkanIncludeDir = Path.GetFullPath(VulkanIncludeDir);
            if (!File.Exists(Path.Combine(VulkanIncludeDir, "vulkan", "vulkan.h")))
                throw new BuildException("MOSS_VULKAN_INCLUDE_DIR must contain vulkan/vulkan.h.");
            PublicSystemIncludePaths.Add(VulkanIncludeDir);
        }

        if (bMossGraphicsVulkan && String.IsNullOrWhiteSpace(VulkanIncludeDir))
            throw new BuildException("MOSS_GRAPHICS_VULKAN requires MOSS_VULKAN_INCLUDE_DIR or VULKAN_SDK.");

        if (bEnableMossGui || bEnableMossXr)
        {
            if (!bHasMossHeaders || !bHasMossLibrary)
                throw new BuildException("MOSS_ENABLE_GUI/XR requires both MOSS_INCLUDE_DIR and MOSS_LIBRARY_FILE.");
        }
        if (bEnableMossGui)
        {
            PublicDefinitions.Remove("MOSSUNREAL_ENABLE_MOSS_GUI=0");
            PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_GUI=1");
        }
        if (bEnableMossXr)
        {
            PublicDefinitions.Remove("MOSSUNREAL_ENABLE_MOSS_XR=0");
            PublicDefinitions.Add("MOSSUNREAL_ENABLE_MOSS_XR=1");
        }

        // Optional transitive libraries for static Moss builds. Supply a
        // semicolon-separated list of absolute library paths.
        string MossExtraLibraries = Environment.GetEnvironmentVariable("MOSS_EXTRA_LIBRARIES");
        if (!String.IsNullOrWhiteSpace(MossExtraLibraries))
        {
            foreach (string ExtraLibrary in MossExtraLibraries.Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries))
            {
                string FullPath = Path.GetFullPath(ExtraLibrary);
                if (!File.Exists(FullPath))
                    throw new BuildException("A file listed in MOSS_EXTRA_LIBRARIES does not exist: " + FullPath);
                PublicAdditionalLibraries.Add(FullPath);
            }
        }
    }

    private static bool ReadBoolEnvironmentVariable(string Name)
    {
        string Value = Environment.GetEnvironmentVariable(Name);
        return String.Equals(Value, "1", StringComparison.OrdinalIgnoreCase)
            || String.Equals(Value, "true", StringComparison.OrdinalIgnoreCase)
            || String.Equals(Value, "yes", StringComparison.OrdinalIgnoreCase);
    }
}
