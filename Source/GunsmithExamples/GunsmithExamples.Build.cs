// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GunsmithExamples : ModuleRules
{
	public GunsmithExamples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Default;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags", 
				"Mover",
				"GameplayAbilities",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Gunsmith",
				"Mover",
				"EnhancedInput",
				"UMG",
				"NetworkPrediction",
				"NetCore", 
				"OnlineSubsystem",
				"AIModule",
				"Sockets",
				"CommonLoadingScreen",
				"ModelViewViewModel", 
				"Niagara",
				"PhysicsCore",
				"DeveloperSettings"
			}
		);

		// Add the itch manifest to the win64 output - linux should work without this
		if (!Target.bBuildEditor && Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeDependencies.Add("$(TargetOutputDir)/.itch.toml",
				"$(ProjectDir)/ThirdParty/Win64/.itch.toml",
				StagedFileType.SystemNonUFS);
		}
	}
}
