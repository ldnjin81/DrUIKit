// Glass 테마가 디자인 시스템 tokens.json 값을 그대로 담는지 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "DrGlassTheme.h"
#include "Misc/AutomationTest.h"
#include "Theme/DrUITheme.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGlassThemeLoadTest, "DrUIGlass.Theme.LoadsTokens", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrGlassThemeLoadTest::RunTest(const FString&) {
  FLinearColor C;
  TestTrue(TEXT("#2563eb"), DrGlassTheme::ParseCssColor(TEXT("#2563eb"), C) && C.ToFColor(true) == FColor(0x25, 0x63, 0xeb));
  TestTrue(TEXT("rgba 알파"), DrGlassTheme::ParseCssColor(TEXT("rgba(255,255,255,0.58)"), C) && FMath::IsNearlyEqual(C.A, .58f));
  FString Error;
  const TArray<UDrUITheme*> Themes = DrGlassTheme::LoadThemes(nullptr, Error);
  TestEqual(TEXT("오류 없음"), Error, FString());
  if (!TestEqual(TEXT("테마 2개"), Themes.Num(), 2)) return false;
  TestEqual(TEXT("첫 테마는 밝은 유리"), Themes[0]->ThemeId, FName(TEXT("glass-light")));
  TestEqual(TEXT("색 23개"), Themes[0]->Colors.Num(), 23);
  TestEqual(TEXT("밝은 accent"), Themes[0]->GetColor(TEXT("accent")).ToFColor(true), FColor(0x25, 0x63, 0xeb));
  TestEqual(TEXT("어두운 accent"), Themes[1]->GetColor(TEXT("accent")).ToFColor(true), FColor(0x6e, 0xa0, 0xff));
  TestTrue(TEXT("glass 반투명"), FMath::IsNearlyEqual(Themes[0]->GetColor(TEXT("glass")).A, .58f));
  TestEqual(TEXT("body 14px → 10.5"), Themes[0]->GetTextStyle(TEXT("body")).Size, 10.5f);
  TestEqual(TEXT("radius-xl"), Themes[0]->GetRadius(TEXT("radius-xl")), 28.f);
  TestTrue(TEXT("흐림 토큰"), Themes[0]->GetBackdrop(TEXT("backdrop-glass")) > 0.f);
  return true;
}

#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUIText.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGlassThemeSwitchTextTest, "DrUIGlass.Theme.TextFollowsThemeSwitch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrGlassThemeSwitchTextTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  FString Error;
  const TArray<UDrUITheme*> Themes = DrGlassTheme::LoadThemes(GetTransientPackage(), Error);
  if (!TestNotNull(TEXT("서브시스템"), Subsystem) || !TestEqual(TEXT("테마 2개"), Themes.Num(), 2)) return false;
  TestEqual(TEXT("어두운 on-accent 값"), Themes[1]->GetColor(TEXT("on-accent")).ToFColor(true), FColor(0x0b, 0x14, 0x30));
  UDrUIText* Text = NewObject<UDrUIText>();
  Text->ColorToken = TEXT("on-accent");
  Subsystem->SetThemeObject(Themes[0]);
  Text->ApplyTheme();
  TestEqual(TEXT("밝은 테마 글자색"), Text->GetColorAndOpacity().GetSpecifiedColor().ToFColor(true), FColor::White);
  Subsystem->SetThemeObject(Themes[1]);
  Text->ApplyTheme();
  TestEqual(TEXT("어두운 테마 글자색"), Text->GetColorAndOpacity().GetSpecifiedColor().ToFColor(true), FColor(0x0b, 0x14, 0x30));
  Subsystem->SetTheme(TEXT("glass-light"));
  return true;
}
#endif
