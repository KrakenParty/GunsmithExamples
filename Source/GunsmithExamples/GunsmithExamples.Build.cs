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
				"Gunsmith",
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
				"DeveloperSettings", 
				"ChaosMover"
			}
		);

		PrivateDefinitions.AddRange(new string[]
		{
			"DEBUG_NETWORK_PHYSICS=1",
			"DEBUG_REWIND_DATA=1"
		});
	}
}
