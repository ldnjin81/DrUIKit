using UnrealBuildTool;

public class DrUIGlass : ModuleRules
{
	public DrUIGlass(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "SlateCore", "Slate", "UMG", "CommonUI", "DrUIKit" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "Projects" });
		// 디자인 시스템 토큰 원본. 패키지에도 함께 넣는다.
		RuntimeDependencies.Add("$(PluginDir)/Tokens/tokens.json");
		// Pretendard(OFL) TTF. 코드에서 파일 경로로 글꼴을 만든다.
		RuntimeDependencies.Add("$(PluginDir)/Fonts/...");
	}
}
