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
	}
}
