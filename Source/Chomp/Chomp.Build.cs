// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Chomp : ModuleRules
{
	public Chomp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
			{ "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "UE5Coro", "RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		// Custom settings for UE5Coro.
		CppStandard = CppStandardVersion.Cpp20;
	}
}