// Copyright Epic Games, Inc. All Rights Reserved.
	 
using UnrealBuildTool;
	 
public class GunsmithExamplesClientTarget : TargetRules
{
	public GunsmithExamplesClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GunsmithExamples");
		
		ProjectDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=3822750");
	}
}