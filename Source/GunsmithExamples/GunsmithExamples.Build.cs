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
				"ModelViewViewModel"
			}
		);
	}
}
