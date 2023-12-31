// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ChompTarget : TargetRules
{
	public ChompTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("Chomp");
		
		// Custom settings for UE5Coro.
		bEnableCppCoroutinesForEvaluation = true;
		CppStandard = CppStandardVersion.Cpp20;
		bOverrideBuildEnvironment = true;
	}
}