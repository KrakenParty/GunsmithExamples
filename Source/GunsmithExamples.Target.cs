// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GunsmithExamplesTarget : TargetRules
{
	public GunsmithExamplesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bWithPushModel = true;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("GunsmithExamples");
	}
}
