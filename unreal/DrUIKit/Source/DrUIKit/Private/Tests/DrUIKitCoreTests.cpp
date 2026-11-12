// 상태 판정, 움직임 계산, 테마 토큰, 표면 품질 단계, 표시 그리기·스위치 손잡이 계산을 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/DrUIMotion.h"
#include "Core/DrUIState.h"
#include "Misc/AutomationTest.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUIChoice.h"
#include "Widgets/DrUIGlyph.h"
#include "Widgets/DrUISurface.h"

constexpr EAutomationTestFlags DrUIKitTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIStatePriorityTest, "DrUIKit.Core.State.Priority", DrUIKitTestFlags)
bool FDrUIStatePriorityTest::RunTest(const FString&) {
  FDrUIStateInputs In;
  TestEqual(TEXT("아무것도 없으면 기본"), DrUIState::Resolve(In), EDrUIState::Default);
  In.bFocused = true;
  TestEqual(TEXT("포커스"), DrUIState::Resolve(In), EDrUIState::Focus);
  In.bSelected = true;
  TestEqual(TEXT("선택됨이 포커스보다 앞"), DrUIState::Resolve(In), EDrUIState::Selected);
  In.bHovered = true;
  TestEqual(TEXT("호버"), DrUIState::Resolve(In), EDrUIState::Hover);
  In.bPressed = true;
  TestEqual(TEXT("눌림"), DrUIState::Resolve(In), EDrUIState::Pressed);
  In.bError = true;
  TestEqual(TEXT("오류"), DrUIState::Resolve(In), EDrUIState::Error);
  In.bSuccess = true;
  TestEqual(TEXT("성공"), DrUIState::Resolve(In), EDrUIState::Success);
  In.bLoading = true;
  TestEqual(TEXT("로딩"), DrUIState::Resolve(In), EDrUIState::Loading);
  TestTrue(TEXT("포커스 링은 상태와 별개"), DrUIState::IsFocusRingVisible(In));
  In.bDisabled = true;
  TestEqual(TEXT("비활성이 가장 앞"), DrUIState::Resolve(In), EDrUIState::Disabled);
  TestFalse(TEXT("비활성이면 포커스 링 숨김"), DrUIState::IsFocusRingVisible(In));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIMotionCurveTest, "DrUIKit.Core.Motion.Curves", DrUIKitTestFlags)
bool FDrUIMotionCurveTest::RunTest(const FString&) {
  TestEqual(TEXT("이즈 아웃 시작"), DrUIMotion::EaseOut(0.f), 0.f);
  TestEqual(TEXT("이즈 아웃 끝"), DrUIMotion::EaseOut(1.f), 1.f);
  TestTrue(TEXT("이즈 아웃은 빨리 출발(절반에서 80% 이상)"), DrUIMotion::EaseOut(.5f) > .8f);
  TestTrue(TEXT("이즈 인은 천천히 출발(절반에서 50% 미만)"), DrUIMotion::EaseIn(.5f) < .5f);
  float Prev = 0.f;
  bool bMonotonic = true;
  for (int32 I = 1; I <= 50; ++I) { const float V = DrUIMotion::EaseOut(I / 50.f); bMonotonic &= V + 1e-4f >= Prev; Prev = V; }
  TestTrue(TEXT("이즈 아웃은 단조 증가"), bMonotonic);
  TestEqual(TEXT("순차 등장 3번째"), DrUIMotion::StaggerDelay(2), .06f, 1e-5f);
  TestEqual(TEXT("순차 등장 상한(8개 이후 같음)"), DrUIMotion::StaggerDelay(20), DrUIMotion::StaggerDelay(7), 1e-5f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIMotionSpringTest, "DrUIKit.Core.Motion.SpringSettles", DrUIKitTestFlags)
bool FDrUIMotionSpringTest::RunTest(const FString&) {
  FDrUISpring Spring;
  Spring.Snap(0.f);
  Spring.Target = 1.f;
  bool bSettled = false;
  float Time = 0.f;
  for (; Time < 3.f && !bSettled; Time += 1.f / 60.f) bSettled = Spring.Step(1.f / 60.f);
  TestTrue(TEXT("3초 안에 멈춤"), bSettled);
  TestEqual(TEXT("목표에 도달"), Spring.Value, 1.f);
  FDrUISpring Big;
  Big.Snap(0.f);
  Big.Target = 1.f;
  Big.Step(.5f);
  TestTrue(TEXT("큰 프레임에도 발산하지 않음"), FMath::Abs(Big.Value) < 2.f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIThemeDefaultTest, "DrUIKit.Core.Theme.DefaultAndMissing", DrUIKitTestFlags)
bool FDrUIThemeDefaultTest::RunTest(const FString&) {
  const UDrUITheme* Theme = UDrUITheme::MakeDefault(nullptr);
  for (FName Token : {DrUITokens::Ground, DrUITokens::Glass, DrUITokens::GlassStrong, DrUITokens::Ink, DrUITokens::Accent, DrUITokens::OnAccent, DrUITokens::Danger, DrUITokens::Focus})
    TestTrue(*FString::Printf(TEXT("기본 테마에 %s"), *Token.ToString()), Theme->Colors.Contains(Token));
  TestTrue(TEXT("기본 테마에 body 글자"), Theme->TextStyles.Contains(DrUITokens::Body));
  AddExpectedMessage(TEXT("no-such-token"), EAutomationExpectedMessageFlags::Contains, 1);
  TestEqual(TEXT("없는 색은 자홍색"), Theme->GetColor(TEXT("no-such-token")), UDrUITheme::MissingColor);
  Theme->GetColor(TEXT("no-such-token"));  // 경고는 한 번만
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISurfaceQualityTest, "DrUIKit.Core.Surface.QualityFallback", DrUIKitTestFlags)
bool FDrUISurfaceQualityTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("테마 서브시스템"), Subsystem)) return false;
  UDrUITheme* Theme = UDrUITheme::MakeDefault(GetTransientPackage());
  Theme->ThemeId = TEXT("test-surface");
  Theme->Colors.Add(DrUITokens::Glass, FLinearColor(1, 1, 1, .5f));
  Theme->Colors.Add(DrUITokens::GlassStrong, FLinearColor(1, 1, 1, .9f));
  Theme->Backdrops.Add(DrUITokens::BackdropGlass, 20.f);
  const EDrUISurfaceQuality Before = Subsystem->GetSurfaceQuality();
  Subsystem->SetThemeObject(Theme);
  UDrUISurface* Surface = NewObject<UDrUISurface>();
  Subsystem->SetSurfaceQuality(EDrUISurfaceQuality::High);
  TestEqual(TEXT("높음: 흐림 그대로"), Surface->ResolveBlur(), 20.f);
  TestEqual(TEXT("높음: 반투명 채움"), Surface->ResolveFill().A, .5f);
  Subsystem->SetSurfaceQuality(EDrUISurfaceQuality::Medium);
  TestEqual(TEXT("보통: 흐림 절반"), Surface->ResolveBlur(), 10.f);
  Subsystem->SetSurfaceQuality(EDrUISurfaceQuality::Low);
  TestEqual(TEXT("낮음: 흐림 없음"), Surface->ResolveBlur(), 0.f);
  TestEqual(TEXT("낮음: 불투명에 가까운 채움"), Surface->ResolveFill().A, .9f);
  Subsystem->SetSurfaceQuality(Before);
  Subsystem->SetTheme(TEXT("default"));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIGlyphTrimTest, "DrUIKit.Core.Glyph.Trim", DrUIKitTestFlags)
bool FDrUIGlyphTrimTest::RunTest(const FString&) {
  const TArray<FVector2f> Line = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}};
  TestEqual(TEXT("0이면 아무것도 그리지 않음"), DrUIGlyph::Trim(Line, 0.f).Num(), 0);
  TestEqual(TEXT("1이면 그대로"), DrUIGlyph::Trim(Line, 1.f).Num(), 3);
  const TArray<FVector2f> Half = DrUIGlyph::Trim(Line, .5f);
  TestEqual(TEXT("절반은 첫 획 끝까지"), Half.Num(), 2);
  TestTrue(TEXT("절반 끝점 (1,0)"), Half.Last().Equals(FVector2f(1.f, 0.f), 1e-4f));
  const TArray<FVector2f> ThreeQuarter = DrUIGlyph::Trim(Line, .75f);
  TestTrue(TEXT("3/4 끝점 (1,.5)"), ThreeQuarter.Last().Equals(FVector2f(1.f, .5f), 1e-4f));
  TestEqual(TEXT("체크는 획 하나"), DrUIGlyph::Strokes(EDrUIGlyphShape::Check).Num(), 1);
  TestEqual(TEXT("닫기는 획 둘"), DrUIGlyph::Strokes(EDrUIGlyphShape::Close).Num(), 2);
  TestEqual(TEXT("점은 획 없음(원)"), DrUIGlyph::Strokes(EDrUIGlyphShape::Dot).Num(), 0);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISwitchThumbTest, "DrUIKit.Core.Switch.ThumbTravel", DrUIKitTestFlags)
bool FDrUISwitchThumbTest::RunTest(const FString&) {
  TestEqual(TEXT("꺼짐은 왼쪽"), UDrUISwitch::ThumbTravel(0.f), 0.f);
  TestEqual(TEXT("켜짐은 트랙 끝에서 여백만큼 안쪽"), UDrUISwitch::ThumbTravel(1.f), UDrUISwitch::TrackWidth - UDrUISwitch::ThumbSize - 2.f * UDrUISwitch::Inset);
  TestEqual(TEXT("스프링이 넘쳐도 트랙 밖으로 안 나감"), UDrUISwitch::ThumbTravel(1.2f), UDrUISwitch::ThumbTravel(1.f));
  return true;
}
#endif
