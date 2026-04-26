// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WU : ModuleRules
{
	public WU(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"HTTP",
			"Json"
		});

		PublicIncludePaths.AddRange(new string[] {
			"WU",
			"WU/Backend",
			"WU/Variant_Combat",
			"WU/Variant_Combat/AI",
			"WU/Variant_Combat/Animation",
			"WU/Variant_Combat/Gameplay",
			"WU/Variant_Combat/Interfaces",
			"WU/Variant_Combat/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
