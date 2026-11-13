// 상태 판정, 움직임 계산, 테마 토큰, 표면 품질 단계·투명도, 표시 그리기·스위치 손잡이·슬라이더 채움, 배지 숫자·진행 링 원호·스켈레톤 깜빡임 계산, 스낵바 차례를 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/DrUIMotion.h"
#include "Core/DrUIState.h"
#include "Misc/AutomationTest.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUIChoice.h"
#include "Widgets/DrUIGlyph.h"
#include "Widgets/DrUINotice.h"
#include "Widgets/DrUISlider.h"
#include "Widgets/DrUIStatus.h"
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISurfaceTransparencyTest, "DrUIKit.Core.Surface.Transparency", DrUIKitTestFlags)
bool FDrUISurfaceTransparencyTest::RunTest(const FString&) {
  TestEqual(TEXT("0은 불투명"), UDrUITheme::AdjustAlpha(.6f, 0.f), 1.f);
  TestEqual(TEXT(".5는 테마 그대로"), UDrUITheme::AdjustAlpha(.6f, .5f), .6f);
  TestEqual(TEXT("1은 1/4"), UDrUITheme::AdjustAlpha(.6f, 1.f), .15f);
  TestEqual(TEXT("범위 밖은 자름"), UDrUITheme::AdjustAlpha(.6f, 2.f), .15f);
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("테마 서브시스템"), Subsystem)) return false;
  UDrUITheme* Theme = UDrUITheme::MakeDefault(GetTransientPackage());
  Theme->ThemeId = TEXT("test-transparency");
  Theme->Colors.Add(DrUITokens::Glass, FLinearColor(1, 1, 1, .5f));
  Theme->Colors.Add(DrUITokens::Accent, FLinearColor(0, 0, 1, 1));
  Theme->Backdrops.Add(DrUITokens::BackdropGlass, 20.f);
  const float Before = Subsystem->GetSurfaceTransparency();
  const EDrUISurfaceQuality QualityBefore = Subsystem->GetSurfaceQuality();
  Subsystem->SetThemeObject(Theme);
  Subsystem->SetSurfaceQuality(EDrUISurfaceQuality::High);
  UDrUISurface* Glass = NewObject<UDrUISurface>();
  UDrUISurface* Solid = NewObject<UDrUISurface>();
  Solid->FillToken = DrUITokens::Accent;
  Subsystem->SetSurfaceTransparency(0.f);
  TestEqual(TEXT("유리는 불투명해짐"), Glass->ResolveFill().A, 1.f);
  Subsystem->SetSurfaceTransparency(1.f);
  TestEqual(TEXT("유리는 더 투명해짐"), Glass->ResolveFill().A, .125f);
  TestEqual(TEXT("TranslucentTokens에 없는 색(accent)은 그대로"), Solid->ResolveFill().A, 1.f);
  Subsystem->SetSurfaceTransparency(Before);
  Subsystem->SetSurfaceQuality(QualityBefore);
  Subsystem->SetTheme(TEXT("default"));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISliderFillTest, "DrUIKit.Core.Slider.FillEnd", DrUIKitTestFlags)
bool FDrUISliderFillTest::RunTest(const FString&) {
  TestEqual(TEXT("0이면 손잡이 반지름까지"), UDrUISlider::FillEnd(200.f, 0.f, 18.f), 9.f);
  TestEqual(TEXT("1이면 끝에서 반지름 안쪽"), UDrUISlider::FillEnd(200.f, 1.f, 18.f), 191.f);
  TestEqual(TEXT("절반은 가운데"), UDrUISlider::FillEnd(200.f, .5f, 18.f), 100.f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIStatusMathTest, "DrUIKit.Core.Status.Math", DrUIKitTestFlags)
bool FDrUIStatusMathTest::RunTest(const FString&) {
  TestTrue(TEXT("배지 0은 빈 글자(숨김)"), UDrUIBadge::FormatCount(0, 99).IsEmpty());
  TestEqual(TEXT("배지 12"), UDrUIBadge::FormatCount(12, 99).ToString(), FString(TEXT("12")));
  TestEqual(TEXT("배지 최댓값 넘으면 99+"), UDrUIBadge::FormatCount(120, 99).ToString(), FString(TEXT("99+")));
  const TArray<FVector2f> Quarter = DrUIProgressRing::ArcPoints(FVector2f(0.f), 10.f, 0.f, .25f, 64);
  TestTrue(TEXT("원호는 12시에서 시작"), Quarter[0].Equals(FVector2f(0.f, -10.f), 1e-3f));
  TestTrue(TEXT("1/4 바퀴는 3시(시계 방향)"), Quarter.Last().Equals(FVector2f(10.f, 0.f), 1e-3f));
  TestEqual(TEXT("1/4 바퀴 조각 16개"), Quarter.Num(), 17);
  TestEqual(TEXT("길이 0이면 점 없음"), DrUIProgressRing::ArcPoints(FVector2f(0.f), 10.f, 0.f, 0.f).Num(), 0);
  float Low = 1.f, High = 0.f;
  for (double T = 0.0; T < 2.4; T += .05) { Low = FMath::Min(Low, UDrUISkeleton::PulseAlpha(T)); High = FMath::Max(High, UDrUISkeleton::PulseAlpha(T)); }
  TestEqual(TEXT("끝없는 진행은 왼쪽에서 시작"), UDrUIProgressBar::MarqueeStart(0.0), 0.f);
  TestTrue(TEXT("끝없는 진행 절반 시점은 오른쪽 끝(.7)"), FMath::IsNearlyEqual(UDrUIProgressBar::MarqueeStart(.8), .7f, 1e-4f));
  TestEqual(TEXT("작은 스피너 조각은 최소 2.5"), UDrUISpinner::AutoPieceSize(4.f), 2.5f);
  TestEqual(TEXT("큰 스피너 조각은 최대 6"), UDrUISpinner::AutoPieceSize(30.f), 6.f);
  TestTrue(TEXT("스켈레톤 불투명도 .18~.32"), Low >= .179f && High <= .321f && High - Low > .1f);
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
  TestEqual(TEXT("켜짐은 트랙 끝에서 여백만큼 안쪽"), UDrUISwitch::ThumbTravel(1.f), DrUISwitchSize::TrackWidth - DrUISwitchSize::ThumbSize - 2.f * DrUISwitchSize::Inset);
  TestEqual(TEXT("스프링이 넘쳐도 트랙 밖으로 안 나감"), UDrUISwitch::ThumbTravel(1.2f), UDrUISwitch::ThumbTravel(1.f));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISnackbarQueueTest, "DrUIKit.Core.Notice.SnackbarQueue", DrUIKitTestFlags)
bool FDrUISnackbarQueueTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("테마 서브시스템"), Subsystem)) return false;
  const bool bWasReduce = Subsystem->IsReduceMotion();
  Subsystem->SetReduceMotion(true);  // 움직임 없이 바로 나타나고 바로 사라지게
  UDrUISnackbar* Bar = NewObject<UDrUISnackbar>(GetTransientPackage());
  const int32 First = Bar->Show(FText::FromString(TEXT("첫째")), FText::FromString(TEXT("되돌리기")), 4.f);
  const int32 Second = Bar->Show(FText::FromString(TEXT("둘째")), FText::GetEmpty(), 0.f);
  TestTrue(TEXT("요청 번호는 차례로"), Second == First + 1);
  TestTrue(TEXT("첫째가 보임"), Bar->IsShown() && Bar->GetCurrentId() == First);
  TestEqual(TEXT("둘째는 기다림"), Bar->GetQueuedCount(), 1);
  Bar->Dismiss();
  TestEqual(TEXT("닫으면 다음 요청"), Bar->GetCurrentId(), Second);
  TestEqual(TEXT("줄이 빔"), Bar->GetQueuedCount(), 0);
  Bar->Dismiss();
  TestFalse(TEXT("다 닫으면 숨김"), Bar->IsShown());
  Subsystem->SetReduceMotion(bWasReduce);

  TSet<EDrUIGlyphShape> Shapes;
  for (const EDrUINoticeTone Tone : {EDrUINoticeTone::Info, EDrUINoticeTone::Success, EDrUINoticeTone::Warning, EDrUINoticeTone::Danger}) {
    EDrUIGlyphShape Shape;
    FName Color;
    UDrUIToast::ToneGlyph(Tone, Shape, Color);
    Shapes.Add(Shape);
  }
  TestEqual(TEXT("토스트 색 4종은 모양도 모두 다름(색만으로 구분하지 않음)"), Shapes.Num(), 4);
  TestEqual(TEXT("느낌표는 획 둘"), DrUIGlyph::Strokes(EDrUIGlyphShape::Alert).Num(), 2);
  return true;
}
#endif
