// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LedgeOut : ModuleRules
{
	public LedgeOut(ReadOnlyTargetRules Target) : base(Target)
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
			"LedgeOut",
			"LedgeOut/Variant_Platforming",
			"LedgeOut/Variant_Platforming/Animation",
			"LedgeOut/Variant_Combat",
			"LedgeOut/Variant_Combat/AI",
			"LedgeOut/Variant_Combat/Animation",
			"LedgeOut/Variant_Combat/Gameplay",
			"LedgeOut/Variant_Combat/Interfaces",
			"LedgeOut/Variant_Combat/UI",
			"LedgeOut/Variant_SideScrolling",
			"LedgeOut/Variant_SideScrolling/AI",
			"LedgeOut/Variant_SideScrolling/Gameplay",
			"LedgeOut/Variant_SideScrolling/Interfaces",
			"LedgeOut/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
