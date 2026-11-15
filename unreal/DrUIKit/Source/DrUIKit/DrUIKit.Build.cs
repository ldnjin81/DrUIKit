using UnrealBuildTool;

public class DrUIKit : ModuleRules
{
	public DrUIKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore",
			"Slate",
			"UMG",
			"CommonUI",
			"CommonInput",
			"EnhancedInput",
			"DeveloperSettings"
		});
		// 스타일 로더(DrUIStyle): tokens.json·style.json 읽기, 플러그인 폴더 찾기.
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "Projects" });
	}
}
