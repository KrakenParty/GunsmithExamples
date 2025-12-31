// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using EpicGames.Core;

public class GunsmithExamplesTarget : TargetRules
{
	[CommandLine("-DisablePushModel")]
	public bool bDisablePushModel { get; set; } = false;
	
	public GunsmithExamplesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bWithPushModel = !bDisablePushModel;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GunsmithExamples");
		
		// Enabled to improve demo QOL
		bUseConsoleInShipping = true;
		bUseLoggingInShipping = true;
		bUseExecCommandsInShipping = true;
		
		ProjectDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=3822750");
	}
}
