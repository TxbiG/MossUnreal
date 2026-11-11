using UnrealBuildTool;

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
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
        });

        // Add the verified Moss include path and library linkage only after
        // selecting a supported Moss distribution/packaging strategy.
    }
}
