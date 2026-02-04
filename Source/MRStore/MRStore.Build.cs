// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MRStore : ModuleRules
{
	public MRStore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 이 모듈을 쓰는 곳에서도 필요한 것
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});

        // 이 모듈 내부에서만 사용하는 것
        //PrivateDependencyModuleNames.AddRange(
        //    new string[]
        //    {
        //        "CoreUObject",
        //        "Engine",
        //    }
        //);
    }
}
