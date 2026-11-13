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
  FDrUIShadow Shadow;
  TestTrue(TEXT("box-shadow: inset 겹은 건너뛰고 바깥 겹"), DrGlassTheme::ParseCssShadow(TEXT("inset 0 1px 0 rgba(255,255,255,0.95), 0 16px 48px -16px rgba(24,32,56,0.28)"), Shadow)
    && Shadow.Offset == FVector2D(0, 16) && Shadow.Blur == 48.f && Shadow.Spread == -16.f && FMath::IsNearlyEqual(Shadow.Color.A, .28f));
  TestTrue(TEXT("그림자 토큰 3개"), Themes[0]->Shadows.Num() == 3 && Themes[1]->GetShadow(TEXT("shadow-float")).Color.A > .5f);
  if (TestTrue(TEXT("Pretendard 글꼴"), Themes[0]->RuntimeFont.IsValid())) {
    TestEqual(TEXT("서체 5개"), Themes[0]->RuntimeFont->DefaultTypeface.Fonts.Num(), 5);
    TestEqual(TEXT("600 → SemiBold"), UDrUITheme::PickTypeface(*Themes[0]->RuntimeFont, 600), FName(TEXT("SemiBold")));
    TestEqual(TEXT("650 → Bold(같은 거리면 굵은 쪽)"), UDrUITheme::PickTypeface(*Themes[0]->RuntimeFont, 650), FName(TEXT("Bold")));
    TestEqual(TEXT("body 글꼴은 Pretendard"), Themes[0]->MakeFont(TEXT("body")).CompositeFont, Themes[0]->RuntimeFont);
    TestFalse(TEXT("code 글꼴은 엔진 기본"), Themes[0]->MakeFont(TEXT("code")).CompositeFont == Themes[0]->RuntimeFont);
  }
  return true;
}

#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrText.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGlassThemeSwitchTextTest, "DrUIGlass.Theme.TextFollowsThemeSwitch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrGlassThemeSwitchTextTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  FString Error;
  const TArray<UDrUITheme*> Themes = DrGlassTheme::LoadThemes(GetTransientPackage(), Error);
  if (!TestNotNull(TEXT("서브시스템"), Subsystem) || !TestEqual(TEXT("테마 2개"), Themes.Num(), 2)) return false;
  TestEqual(TEXT("어두운 on-accent 값"), Themes[1]->GetColor(TEXT("on-accent")).ToFColor(true), FColor(0x0b, 0x14, 0x30));
  UDrText* Text = NewObject<UDrText>();
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
