// 상태 판정, 움직임 계산, 테마 토큰, 표면 품질 단계·투명도, 표시 그리기·스위치 손잡이·슬라이더 채움, 배지 숫자·진행 링 원호·스켈레톤 깜빡임 계산, 스낵바 차례, 입력 글자 수 제한·수평 선택기 이동, 파이 메뉴 방향, 페이지 점 배치, 버튼 프롬프트 키 이름, 순차 등장 진행, 쿨다운 표시를 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/DrUIMotion.h"
#include "Core/DrUIStagger.h"
#include "Core/DrUIState.h"
#include "Misc/AutomationTest.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrToggle.h"
#include "Widgets/DrButtonPrompt.h"
#include "Widgets/DrCooldown.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrInput.h"
#include "Widgets/DrNotice.h"
#include "Widgets/DrPageIndicator.h"
#include "Widgets/DrRadialMenu.h"
#include "Widgets/DrSlider.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"

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
  UDrSurface* Surface = NewObject<UDrSurface>();
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
  UDrSurface* Glass = NewObject<UDrSurface>();
  UDrSurface* Solid = NewObject<UDrSurface>();
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
  TestEqual(TEXT("0이면 손잡이 반지름까지"), UDrSlider::FillEnd(200.f, 0.f, 18.f), 9.f);
  TestEqual(TEXT("1이면 끝에서 반지름 안쪽"), UDrSlider::FillEnd(200.f, 1.f, 18.f), 191.f);
  TestEqual(TEXT("절반은 가운데"), UDrSlider::FillEnd(200.f, .5f, 18.f), 100.f);
  TestEqual(TEXT("0~1을 .25씩이면 눈금 4칸"), UDrSlider::TickSteps(0.f, 1.f, .25f, 12), 4);
  TestEqual(TEXT("0~100을 10씩이면 10칸"), UDrSlider::TickSteps(0.f, 100.f, 10.f, 12), 10);
  TestEqual(TEXT("촘촘하면(20칸) 눈금 없음"), UDrSlider::TickSteps(0.f, 1.f, .05f, 12), 0);
  TestEqual(TEXT("범위를 딱 나누지 않으면 눈금 없음"), UDrSlider::TickSteps(0.f, 1.f, .3f, 12), 0);
  TestEqual(TEXT("단계가 없으면 눈금 없음"), UDrSlider::TickSteps(0.f, 1.f, 0.f, 12), 0);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIStatusMathTest, "DrUIKit.Core.Status.Math", DrUIKitTestFlags)
bool FDrUIStatusMathTest::RunTest(const FString&) {
  TestTrue(TEXT("배지 0은 빈 글자(숨김)"), UDrBadge::FormatCount(0, 99).IsEmpty());
  TestEqual(TEXT("배지 12"), UDrBadge::FormatCount(12, 99).ToString(), FString(TEXT("12")));
  TestEqual(TEXT("배지 최댓값 넘으면 99+"), UDrBadge::FormatCount(120, 99).ToString(), FString(TEXT("99+")));
  const TArray<FVector2f> Quarter = DrProgressRing::ArcPoints(FVector2f(0.f), 10.f, 0.f, .25f, 64);
  TestTrue(TEXT("원호는 12시에서 시작"), Quarter[0].Equals(FVector2f(0.f, -10.f), 1e-3f));
  TestTrue(TEXT("1/4 바퀴는 3시(시계 방향)"), Quarter.Last().Equals(FVector2f(10.f, 0.f), 1e-3f));
  TestEqual(TEXT("1/4 바퀴 조각 16개"), Quarter.Num(), 17);
  TestEqual(TEXT("길이 0이면 점 없음"), DrProgressRing::ArcPoints(FVector2f(0.f), 10.f, 0.f, 0.f).Num(), 0);
  float Low = 1.f, High = 0.f;
  for (double T = 0.0; T < 2.4; T += .05) { Low = FMath::Min(Low, UDrSkeleton::PulseAlpha(T)); High = FMath::Max(High, UDrSkeleton::PulseAlpha(T)); }
  TestEqual(TEXT("끝없는 진행은 왼쪽에서 시작"), UDrProgressBar::MarqueeStart(0.0), 0.f);
  TestTrue(TEXT("끝없는 진행 절반 시점은 오른쪽 끝(.7)"), FMath::IsNearlyEqual(UDrProgressBar::MarqueeStart(.8), .7f, 1e-4f));
  TestEqual(TEXT("작은 스피너 조각은 최소 2.5"), UDrSpinner::AutoPieceSize(4.f), 2.5f);
  TestEqual(TEXT("큰 스피너 조각은 최대 6"), UDrSpinner::AutoPieceSize(30.f), 6.f);
  TestTrue(TEXT("스켈레톤 불투명도 .18~.32"), Low >= .179f && High <= .321f && High - Low > .1f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIGlyphTrimTest, "DrUIKit.Core.Glyph.Trim", DrUIKitTestFlags)
bool FDrUIGlyphTrimTest::RunTest(const FString&) {
  const TArray<FVector2f> Line = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}};
  TestEqual(TEXT("0이면 아무것도 그리지 않음"), DrGlyph::Trim(Line, 0.f).Num(), 0);
  TestEqual(TEXT("1이면 그대로"), DrGlyph::Trim(Line, 1.f).Num(), 3);
  const TArray<FVector2f> Half = DrGlyph::Trim(Line, .5f);
  TestEqual(TEXT("절반은 첫 획 끝까지"), Half.Num(), 2);
  TestTrue(TEXT("절반 끝점 (1,0)"), Half.Last().Equals(FVector2f(1.f, 0.f), 1e-4f));
  const TArray<FVector2f> ThreeQuarter = DrGlyph::Trim(Line, .75f);
  TestTrue(TEXT("3/4 끝점 (1,.5)"), ThreeQuarter.Last().Equals(FVector2f(1.f, .5f), 1e-4f));
  TestEqual(TEXT("체크는 획 하나"), DrGlyph::Strokes(EDrGlyphShape::Check).Num(), 1);
  TestEqual(TEXT("닫기는 획 둘"), DrGlyph::Strokes(EDrGlyphShape::Close).Num(), 2);
  TestEqual(TEXT("점은 획 없음(원)"), DrGlyph::Strokes(EDrGlyphShape::Dot).Num(), 0);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISwitchThumbTest, "DrUIKit.Core.Switch.ThumbTravel", DrUIKitTestFlags)
bool FDrUISwitchThumbTest::RunTest(const FString&) {
  TestEqual(TEXT("꺼짐은 왼쪽"), UDrSwitch::ThumbTravel(0.f), 0.f);
  TestEqual(TEXT("켜짐은 트랙 끝에서 여백만큼 안쪽"), UDrSwitch::ThumbTravel(1.f), DrSwitchSize::TrackWidth - DrSwitchSize::ThumbSize - 2.f * DrSwitchSize::Inset);
  TestEqual(TEXT("스프링이 넘쳐도 트랙 밖으로 안 나감"), UDrSwitch::ThumbTravel(1.2f), UDrSwitch::ThumbTravel(1.f));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISnackbarQueueTest, "DrUIKit.Core.Notice.SnackbarQueue", DrUIKitTestFlags)
bool FDrUISnackbarQueueTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("테마 서브시스템"), Subsystem)) return false;
  const bool bWasReduce = Subsystem->IsReduceMotion();
  Subsystem->SetReduceMotion(true);  // 움직임 없이 바로 나타나고 바로 사라지게
  UDrSnackbar* Bar = NewObject<UDrSnackbar>(GetTransientPackage());
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

  TSet<EDrGlyphShape> Shapes;
  for (const EDrNoticeTone Tone : {EDrNoticeTone::Info, EDrNoticeTone::Success, EDrNoticeTone::Warning, EDrNoticeTone::Danger}) {
    EDrGlyphShape Shape;
    FName Color;
    UDrToast::ToneGlyph(Tone, Shape, Color);
    Shapes.Add(Shape);
  }
  TestEqual(TEXT("토스트 색 4종은 모양도 모두 다름(색만으로 구분하지 않음)"), Shapes.Num(), 4);
  TestEqual(TEXT("느낌표는 획 둘"), DrGlyph::Strokes(EDrGlyphShape::Alert).Num(), 2);
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrInputMathTest, "DrUIKit.Core.Input.Math", DrUIKitTestFlags)
bool FDrInputMathTest::RunTest(const FString&) {
  TestEqual(TEXT("제한 없음(0)"), UDrTextField::ClampLength(FText::FromString(TEXT("안녕하세요")), 0).ToString(), FString(TEXT("안녕하세요")));
  TestEqual(TEXT("넘으면 자름"), UDrTextField::ClampLength(FText::FromString(TEXT("안녕하세요")), 3).ToString(), FString(TEXT("안녕하")));
  TestEqual(TEXT("다음(감쌈) 끝 → 처음"), UDrHorizontalSelector::StepIndex(2, 1, 3, true), 0);
  TestEqual(TEXT("이전(감쌈) 처음 → 끝"), UDrHorizontalSelector::StepIndex(0, -1, 3, true), 2);
  TestEqual(TEXT("감싸지 않으면 끝에서 멈춤"), UDrHorizontalSelector::StepIndex(2, 1, 3, false), 2);
  TestEqual(TEXT("감싸지 않으면 처음에서 멈춤"), UDrHorizontalSelector::StepIndex(0, -1, 3, false), 0);
  TestEqual(TEXT("고른 것 없으면 다음은 처음"), UDrHorizontalSelector::StepIndex(INDEX_NONE, 1, 3, true), 0);
  TestEqual(TEXT("항목이 없으면 없음"), UDrHorizontalSelector::StepIndex(0, 1, 0, true), int32(INDEX_NONE));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrRadialSectorTest, "DrUIKit.Core.RadialMenu.Sector", DrUIKitTestFlags)
bool FDrRadialSectorTest::RunTest(const FString&) {
  TestEqual(TEXT("위(12시)는 0번"), UDrRadialMenu::SectorAt(FVector2D(0, -1), 4), 0);
  TestEqual(TEXT("오른쪽은 1번(시계 방향)"), UDrRadialMenu::SectorAt(FVector2D(1, 0), 4), 1);
  TestEqual(TEXT("아래는 2번"), UDrRadialMenu::SectorAt(FVector2D(0, 1), 4), 2);
  TestEqual(TEXT("왼쪽은 3번"), UDrRadialMenu::SectorAt(FVector2D(-1, 0), 4), 3);
  TestEqual(TEXT("칸 경계 조금 왼쪽(11시 반)은 0번"), UDrRadialMenu::SectorAt(FVector2D(-.3, -1), 4), 0);
  TestEqual(TEXT("길이 0은 없음"), UDrRadialMenu::SectorAt(FVector2D::ZeroVector, 4), int32(INDEX_NONE));
  TestEqual(TEXT("칸이 없으면 없음"), UDrRadialMenu::SectorAt(FVector2D(1, 0), 0), int32(INDEX_NONE));
  TestTrue(TEXT("5에서 0으로는 6으로(앞으로 한 칸)"), FMath::IsNearlyEqual(UDrRadialMenu::NearestWrapped(5.f, 0, 6), 6.f));
  TestTrue(TEXT("0에서 5로는 -1로(뒤로 한 칸)"), FMath::IsNearlyEqual(UDrRadialMenu::NearestWrapped(0.f, 5, 6), -1.f));
  TestTrue(TEXT("가까우면 그대로"), FMath::IsNearlyEqual(UDrRadialMenu::NearestWrapped(1.f, 2, 6), 2.f));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrPageIndicatorLayoutTest, "DrUIKit.Core.PageIndicator.Layout", DrUIKitTestFlags)
bool FDrPageIndicatorLayoutTest::RunTest(const FString&) {
  using namespace DrPageIndicatorLayout;
  TestEqual(TEXT("지금 페이지 점은 늘어남"), DotWidth(2, 2.f, 8.f, 22.f), 22.f);
  TestEqual(TEXT("먼 점은 그대로"), DotWidth(0, 2.f, 8.f, 22.f), 8.f);
  TestEqual(TEXT("옮겨 가는 중간이면 두 점이 반씩"), DotWidth(1, 1.5f, 8.f, 22.f), 15.f);
  TestEqual(TEXT("전체 너비"), TotalWidth(5, 8.f, 22.f, 8.f), 4.f * 16.f + 22.f);
  float Sum = 0.f;
  for (int32 I = 0; I < 5; ++I) Sum += DotWidth(I, 1.5f, 8.f, 22.f) + (I < 4 ? 8.f : 0.f);
  TestTrue(TEXT("옮겨 가는 중에도 전체 너비가 같음(흔들리지 않음)"), FMath::IsNearlyEqual(Sum, TotalWidth(5, 8.f, 22.f, 8.f)));
  TestEqual(TEXT("점이 없으면 0"), TotalWidth(0, 8.f, 22.f, 8.f), 0.f);
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrButtonPromptKeyTest, "DrUIKit.Core.ButtonPrompt.KeyNames", DrUIKitTestFlags)
bool FDrButtonPromptKeyTest::RunTest(const FString&) {
  TestEqual(TEXT("패드 아래 얼굴 버튼은 A"), UDrButtonPrompt::ShortKeyName(EKeys::Gamepad_FaceButton_Bottom).ToString(), FString(TEXT("A")));
  TestEqual(TEXT("왼쪽 트리거는 LT"), UDrButtonPrompt::ShortKeyName(EKeys::Gamepad_LeftTrigger).ToString(), FString(TEXT("LT")));
  TestEqual(TEXT("Escape는 Esc"), UDrButtonPrompt::ShortKeyName(EKeys::Escape).ToString(), FString(TEXT("Esc")));
  TestEqual(TEXT("글자 키는 엔진 표시 이름"), UDrButtonPrompt::ShortKeyName(EKeys::E).ToString(), FString(TEXT("E")));
  TestTrue(TEXT("얼굴 버튼은 원"), UDrButtonPrompt::IsRoundKey(EKeys::Gamepad_FaceButton_Right));
  TestFalse(TEXT("어깨 버튼은 각진 칩"), UDrButtonPrompt::IsRoundKey(EKeys::Gamepad_LeftShoulder));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrStaggerTest, "DrUIKit.Core.Motion.Stagger", DrUIKitTestFlags)
bool FDrStaggerTest::RunTest(const FString&) {
  using namespace DrUIMotion;
  TestEqual(TEXT("시작 전은 0"), StaggerProgress(0.f, 0), 0.f);
  TestEqual(TEXT("첫 항목은 길이만큼 지나면 끝"), StaggerProgress(.28f, 0), 1.f);
  TestEqual(TEXT("두 번째는 한 단계 늦게 시작"), StaggerProgress(.04f, 1), 0.f);
  TestTrue(TEXT("앞 항목이 늘 더 많이 진행"), StaggerProgress(.15f, 0) > StaggerProgress(.15f, 3));
  TestEqual(TEXT("MaxItems 뒤 항목은 같은 지연"), StaggerProgress(.3f, 20), StaggerProgress(.3f, 7));
  TestTrue(TEXT("이즈 아웃: 절반 시간에 절반보다 많이"), StaggerProgress(.14f, 0) > .5f);
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrCooldownTest, "DrUIKit.Core.Cooldown.Math", DrUIKitTestFlags)
bool FDrCooldownTest::RunTest(const FString&) {
  TestEqual(TEXT("10초 이상은 올림 정수"), UDrCooldown::FormatRemaining(12.2f).ToString(), FString(TEXT("13")));
  TestEqual(TEXT("10초 미만은 소수 한 자리(올림)"), UDrCooldown::FormatRemaining(3.21f).ToString(), FString(TEXT("3.3")));
  TestTrue(TEXT("끝나면 빈 글"), UDrCooldown::FormatRemaining(0.f).IsEmpty());
  TestEqual(TEXT("비율"), UDrCooldown::Fraction(15.f, 60.f), .25f);
  TestEqual(TEXT("다 걷히면 꼭짓점 없음"), UDrCooldown::SweepPolygon(FVector2f(40.f), 0.f).Num(), 0);
  const TArray<FVector2f> Full = UDrCooldown::SweepPolygon(FVector2f(40.f, 20.f), 1.f);
  TestTrue(TEXT("첫 점은 가운데"), Full[0].Equals(FVector2f(20.f, 10.f), 1e-3f));
  TestTrue(TEXT("다 덮으면 12시에서 시작(위 변 가운데)"), Full[1].Equals(FVector2f(20.f, 0.f), 1e-2f));
  bool bOnEdge = true;
  for (int32 I = 1; I < Full.Num(); ++I)
    bOnEdge &= FMath::IsNearlyEqual(Full[I].X, 0.f, .01f) || FMath::IsNearlyEqual(Full[I].X, 40.f, .01f) || FMath::IsNearlyEqual(Full[I].Y, 0.f, .01f) || FMath::IsNearlyEqual(Full[I].Y, 20.f, .01f);
  TestTrue(TEXT("부채꼴 테두리 점은 모두 사각형 변 위"), bOnEdge);
  return true;
}
#endif
