using UnrealBuildTool;

public class DrUIGlass : ModuleRules
{
	public DrUIGlass(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "SlateCore", "Slate", "UMG", "CommonUI", "DrUIKit" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "Projects" });
		// 디자인 시스템 토큰 원본. 패키지에도 함께 넣는다.
		RuntimeDependencies.Add("$(PluginDir)/Tokens/tokens.json");
	}
}
