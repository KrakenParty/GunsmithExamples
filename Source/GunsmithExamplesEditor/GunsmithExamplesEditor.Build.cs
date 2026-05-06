using UnrealBuildTool;

public class GunsmithExamplesEditor : ModuleRules
{
    public GunsmithExamplesEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "Gunsmith",
                "GunsmithExamples",
                "NetworkPrediction", 
                "GunsmithEditor"
            }
        );
    }
}