// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MR : ModuleRules
{
	public MR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "MRStore", "UMG", "GameplayAbilities", "GameplayTags", "GameplayTasks" });

        PrivateIncludePaths.AddRange(new string[] { "MR", "MR/Subsystem", "MR/Widget", "MR/Store", "MR/Action", "MR/Character", "MR/Component", "MR/Animation", "MR/GAS", "MR/GAS/Attribute", "MR/GAS/Ability" });

    }
}
