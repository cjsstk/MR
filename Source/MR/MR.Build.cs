// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MR : ModuleRules
{
	public MR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "MRStore" });
	}
}
