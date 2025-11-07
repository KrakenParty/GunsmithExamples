// Copyright Epic Games, Inc. All Rights Reserved.
	 
using UnrealBuildTool;
	 
public class GunsmithExamplesServerTarget : TargetRules
{
	public GunsmithExamplesServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GunsmithExamples");
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "GunsmithExamplesEditor" });
	}
}