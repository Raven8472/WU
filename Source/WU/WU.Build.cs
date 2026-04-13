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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"WU",
			"WU/Variant_Platforming",
			"WU/Variant_Platforming/Animation",
			"WU/Variant_Combat",
			"WU/Variant_Combat/AI",
			"WU/Variant_Combat/Animation",
			"WU/Variant_Combat/Gameplay",
			"WU/Variant_Combat/Interfaces",
			"WU/Variant_Combat/UI",
			"WU/Variant_SideScrolling",
			"WU/Variant_SideScrolling/AI",
			"WU/Variant_SideScrolling/Gameplay",
			"WU/Variant_SideScrolling/Interfaces",
			"WU/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
