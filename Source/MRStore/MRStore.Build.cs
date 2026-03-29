// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MRStore : ModuleRules
{
	public MRStore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 외부에 공개되는 종속성
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UMG" });

        // 이 모듈 내부에서만 사용하는 종속성
        //PrivateDependencyModuleNames.AddRange(
        //    new string[]
        //    {
        //        "CoreUObject",
        //        "Engine",
        //    }
        //);
    }
}
