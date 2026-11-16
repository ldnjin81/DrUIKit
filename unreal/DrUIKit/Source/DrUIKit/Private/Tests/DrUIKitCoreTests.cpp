// 상태 판정, C++ 버튼이 CommonUI 내부 버튼으로 감싸지는지(마우스 누름), 움직임 계산, 테마 토큰, 스타일 로더(CSS 값 읽기·불러온 스타일이 모든 토큰을 가짐), 표면 품질 단계·투명도·테두리 두께, 표시 그리기·스위치 손잡이·슬라이더 채움, 배지 숫자·진행 링 원호·스켈레톤 깜빡임 계산, 스낵바 차례, 게임패드(입력 뜻·패드별 키 이름·키 모양·포커스 링·칸 격자 넘김·능력치 비교·피드백 세기), 입력 글자 수 제한·수평 선택기 이동, 파이 메뉴 방향, 페이지 점 배치, 버튼 프롬프트 키 이름, 순차 등장 진행, 쿨다운 표시, 글자 등장·데이터 표시 계산을 검증한다.
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/DrUIFeedback.h"
#include "Core/DrUIInput.h"
#include "Core/DrUIMotion.h"
#include "Core/DrUIStagger.h"
#include "Core/DrUIState.h"
#include "Misc/AutomationTest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Theme/DrUIGlyphImages.h"
#include "Theme/DrUIStyle.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrToggle.h"
#include "Widgets/DrButtonPrompt.h"
#include "Widgets/DrConfirmDialog.h"
#include "Widgets/DrCooldown.h"
#include "Widgets/DrData.h"
#include "Widgets/DrFeedback.h"
#include "Widgets/DrActionBar.h"
#include "Widgets/DrOptionRow.h"
#include "Widgets/DrPressAnyKey.h"
#include "Widgets/DrItemGrid.h"
#include "Widgets/DrItemSlot.h"
#include "Widgets/DrStatCompare.h"
#include "Widgets/DrKeyGlyph.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrInput.h"
#include "Widgets/DrNavigation.h"
#include "Widgets/DrNotice.h"
#include "Widgets/DrOverlays.h"
#include "Widgets/DrPageIndicator.h"
#include "Widgets/DrQuantityStepper.h"
#include "Widgets/DrRadialMenu.h"
#include "Widgets/DrSlider.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrTextReveal.h"
#include "Widgets/DrSurface.h"
#include "Blueprint/WidgetTree.h"
#include "UObject/UObjectIterator.h"

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
  TestEqual(TEXT("수량 스테퍼: 0.4초 전에는 반복 없음"), UDrQuantityStepper::RepeatInterval(.3f), 0.f);
  TestTrue(TEXT("수량 스테퍼: 오래 누를수록 빨라짐"), UDrQuantityStepper::RepeatInterval(.5f) > UDrQuantityStepper::RepeatInterval(3.f));
  TestEqual(TEXT("수량 스테퍼: 가장 빠르면 0.04초"), UDrQuantityStepper::RepeatInterval(10.f), .04f);
  TestEqual(TEXT("독: 가리킨 아이콘 1.4배"), UDrDock::MagnifyFor(0), 1.4f);
  TestEqual(TEXT("독: 이웃 1.15배"), UDrDock::MagnifyFor(1), 1.15f);
  TestEqual(TEXT("독: 그 밖은 그대로"), UDrDock::MagnifyFor(3), 1.f);
  TestTrue(TEXT("바텀시트: 높이 25% 넘게 끌면 닫힘"), UDrBottomSheet::ShouldDismissDrag(120.f, 400.f) && !UDrBottomSheet::ShouldDismissDrag(80.f, 400.f));
  TestEqual(TEXT("코치마크: 아래 자리가 있으면 구멍 아래"), UDrCoachMark::PlaceBubble(FVector2D(100, 100), FVector2D(50, 40), FVector2D(200, 100), FVector2D(800, 600)), FVector2D(100, 152));
  TestEqual(TEXT("코치마크: 아래 자리가 없으면 위"), UDrCoachMark::PlaceBubble(FVector2D(100, 520), FVector2D(50, 40), FVector2D(200, 100), FVector2D(800, 600)).Y, 408.0);
  TestEqual(TEXT("코치마크: 오른쪽 끝을 넘지 않음"), UDrCoachMark::PlaceBubble(FVector2D(700, 100), FVector2D(50, 40), FVector2D(200, 100), FVector2D(800, 600)).X, 592.0);
  TestEqual(TEXT("단계 표시: 첫 원은 반 칸 안쪽"), UDrStepIndicator::StepCenterX(0, 4, 400.f), 50.f);
  TestEqual(TEXT("단계 표시: 마지막 원"), UDrStepIndicator::StepCenterX(3, 4, 400.f), 350.f);
  const FVector3f End = UDrCompletion::Timeline(2.f);
  TestTrue(TEXT("완료 표시: 끝나면 원 1·체크 다 그림·고리 다 퍼짐"), FMath::IsNearlyEqual(End.X, 1.f) && FMath::IsNearlyEqual(End.Y, 1.f) && FMath::IsNearlyEqual(End.Z, 1.f));
  TestTrue(TEXT("완료 표시: 원이 살짝 튐(1.1)"), FMath::IsNearlyEqual(UDrCompletion::Timeline(.22f).X, 1.1f, 1e-3f));
  TestEqual(TEXT("완료 표시: 처음엔 체크 없음"), UDrCompletion::Timeline(.1f).Y, 0.f);
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrRevealDataTest, "DrUIKit.Core.RevealAndData", DrUIKitTestFlags)
bool FDrRevealDataTest::RunTest(const FString&) {
  TestEqual(TEXT("타이핑: 초당 20자면 0.5초에 10자"), UDrTextReveal::RevealedCount(.5f, 30, 20.f), 10);
  TestEqual(TEXT("타이핑: 길이를 넘지 않음"), UDrTextReveal::RevealedCount(9.f, 30, 20.f), 30);
  const FString Full = TEXT("다시 오셨네요 Hello");
  const FString Frame = UDrTextReveal::ScrambleFrame(Full, 3, 7u);
  TestEqual(TEXT("스크램블: 길이 같음"), Frame.Len(), Full.Len());
  TestTrue(TEXT("스크램블: 앞 3글자는 그대로"), Frame.Left(3) == Full.Left(3));
  TestEqual(TEXT("스크램블: 띄어쓰기는 그대로"), Frame[7], TCHAR(' '));
  TestTrue(TEXT("스크램블: 한글 자리는 한글"), Frame[4] >= 0xAC00 && Frame[4] <= 0xD7A3);
  const TArray<FVector2f> Spark = UDrSparkline::ToPoints({1.f, 5.f, 3.f}, FVector2f(100.f, 40.f), 0.f);
  TestTrue(TEXT("스파크라인: 큰 값이 위, 작은 값이 아래, 양 끝 x"), Spark[1].Y == 0.f && Spark[0].Y == 40.f && Spark[2].X == 100.f);
  const TArray<FVector2f> Segs = UDrDonut::Segments({1.f, 3.f, -2.f});
  TestTrue(TEXT("도넛: 비율(음수는 0)"), Segs.Num() == 3 && FMath::IsNearlyEqual(Segs[1].X, .25f) && FMath::IsNearlyEqual(Segs[1].Y, .75f) && Segs[2].Y == 0.f);
  TestEqual(TEXT("통계 카드: 오름"), UDrStatCard::FormatDelta(.12f).ToString(), FString(TEXT("↑12%")));
  TestEqual(TEXT("통계 카드: 변화 없음"), UDrStatCard::FormatDelta(0.f).ToString(), FString(TEXT("—")));
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrConfirmDialogTest, "DrUIKit.Core.Flow.ConfirmResolvesOnce", DrUIKitTestFlags)
bool FDrConfirmDialogTest::RunTest(const FString&) {
  UDrConfirmDialog* Dialog = NewObject<UDrConfirmDialog>(GetTransientPackage());
  int32 Calls = 0;
  bool bLast = true;
  Dialog->SetContent(FText::FromString(TEXT("삭제할까요?")), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty(), true);
  Dialog->OnResultNative = [&](bool bOk) { ++Calls; bLast = bOk; };
  Dialog->Resolve(false);
  Dialog->Resolve(true);  // 이미 결정됐으면 무시
  TestEqual(TEXT("결과는 한 번만 알린다"), Calls, 1);
  TestFalse(TEXT("첫 결정(취소)이 남는다"), bLast);
  return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrButtonWrappedTest, "DrUIKit.Core.Button.WrappedForMouse", DrUIKitTestFlags)
bool FDrButtonWrappedTest::RunTest(const FString&) {
  // C++ 기본 구성이 CommonUI 내부 버튼 안에 있어야 마우스 누름이 닿는다(11/13: 갤러리에서 슬라이더 말고는 눌리지 않았다).
  int32 Checked = 0;
  for (TObjectIterator<UClass> It; It; ++It) {
    UClass* Class = *It;
    if (!Class->IsChildOf(UDrButtonBase::StaticClass()) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) || !Class->IsNative()) continue;
    UDrButtonBase* Button = NewObject<UDrButtonBase>(GetTransientPackage(), Class);
    Button->Initialize();
    const UWidget* Root = Button->WidgetTree ? Button->WidgetTree->RootWidget.Get() : nullptr;
    TestTrue(FString::Printf(TEXT("%s 루트가 내부 버튼"), *Class->GetName()), Root && Root->IsA<UCommonButtonInternalBase>());
    ++Checked;
  }
  TestTrue(TEXT("검사한 버튼 클래스가 있다"), Checked > 5);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIStyleCssTest, "DrUIKit.Style.ParseCss", DrUIKitTestFlags)
bool FDrUIStyleCssTest::RunTest(const FString&) {
  FLinearColor C;
  TestTrue(TEXT("#2563eb"), DrUIStyle::ParseCssColor(TEXT("#2563eb"), C) && C.ToFColor(true) == FColor(0x25, 0x63, 0xeb));
  TestTrue(TEXT("rgba 알파"), DrUIStyle::ParseCssColor(TEXT("rgba(255,255,255,0.58)"), C) && FMath::IsNearlyEqual(C.A, .58f));
  TestTrue(TEXT("#00000000은 투명"), DrUIStyle::ParseCssColor(TEXT("#00000000"), C) && C.A == 0.f);
  FDrUIShadow Shadow;
  TestTrue(TEXT("inset 겹은 건너뛰고 바깥 겹"), DrUIStyle::ParseCssShadow(TEXT("inset 0 1px 0 rgba(255,255,255,0.95), 0 16px 48px -16px rgba(24,32,56,0.28)"), Shadow)
    && Shadow.Offset == FVector2D(0, 16) && Shadow.Blur == 48.f && Shadow.Spread == -16.f && FMath::IsNearlyEqual(Shadow.Color.A, .28f));
  const TArray<FDrUIShadow> Two = DrUIStyle::ParseCssShadows(TEXT("-6px -6px 14px rgba(255,255,255,0.9), 6px 6px 16px rgba(150,163,184,0.6)"));
  TestTrue(TEXT("두 겹(뉴모피즘): 밝은 겹은 왼쪽 위"), Two.Num() == 2 && Two[0].Offset == FVector2D(-6, -6) && Two[1].Offset == FVector2D(6, 6));
  TestTrue(TEXT("흐림 없는 겹(브루탈리즘)"), DrUIStyle::ParseCssShadow(TEXT("4px 4px 0 #111111"), Shadow) && Shadow.Blur == 0.f && Shadow.Color.A == 1.f);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUIStylesCompleteTest, "DrUIKit.Style.LoadedStylesComplete", DrUIKitTestFlags)
bool FDrUIStylesCompleteTest::RunTest(const FString&) {
  // 켜진 스타일 플러그인을 모두 다시 읽어, 첫 스타일(order가 가장 작은 것)의 첫 테마가 가진 토큰을 모든 테마가 다 갖는지 본다.
  const TArray<FString> Plugins = DrUIStyle::FindStylePlugins();
  AddInfo(FString::Printf(TEXT("스타일 플러그인 %d개: %s"), Plugins.Num(), *FString::Join(Plugins, TEXT(", "))));
  TestEqual(TEXT("시작할 때 불러온 스타일 수"), DrUIStyle::GetLoadedStyles().Num(), Plugins.Num());
  const UDrUITheme* Reference = nullptr;
  TSet<FName> StyleIds;
  for (const FString& Plugin : Plugins) {
    FString Error;
    FDrUIStyleInfo Info;
    const TArray<UDrUITheme*> Themes = DrUIStyle::LoadPluginStyle(Plugin, GetTransientPackage(), Error, &Info);
    TestEqual(Plugin + TEXT(" 오류 없음"), Error, FString());
    TestFalse(Plugin + TEXT(" 스타일 id가 겹치지 않음"), StyleIds.Contains(Info.Id));
    StyleIds.Add(Info.Id);
    if (!TestEqual(Plugin + TEXT(" 밝게·어둡게 두 테마"), Themes.Num(), 2)) continue;
    if (!Reference) Reference = Themes[0];
    for (const UDrUITheme* Theme : Themes) {
      const FString Id = Theme->ThemeId.ToString();
      TestTrue(Id + TEXT(" 테마 id는 스타일 id로 시작"), Id.StartsWith(Info.Id.ToString() + TEXT("-")));
      auto Keys = [this, &Id](const TCHAR* Kind, const auto& Want, const auto& Have) {
        for (const auto& Pair : Want) TestTrue(FString::Printf(TEXT("%s %s 토큰 %s"), *Id, Kind, *Pair.Key.ToString()), Have.Contains(Pair.Key));
      };
      Keys(TEXT("색"), Reference->Colors, Theme->Colors);
      Keys(TEXT("글자"), Reference->TextStyles, Theme->TextStyles);
      Keys(TEXT("간격"), Reference->Spacing, Theme->Spacing);
      Keys(TEXT("모서리"), Reference->Radius, Theme->Radius);
      Keys(TEXT("그림자"), Reference->Shadows, Theme->Shadows);
      Keys(TEXT("흐림"), Reference->Backdrops, Theme->Backdrops);
      if (TestTrue(Id + TEXT(" 본문 글꼴"), Theme->RuntimeFont.IsValid()))
        TestEqual(Id + TEXT(" body는 스타일 글꼴"), Theme->MakeFont(DrUITokens::Body).CompositeFont, Theme->RuntimeFont);
    }
    if (Info.Id == TEXT("neu")) TestEqual(TEXT("뉴모피즘 shadow-glass는 두 겹"), Themes[0]->GetShadowLayers(DrUITokens::ShadowGlass).Num(), 2);
  }
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrUISurfaceStrokeTest, "DrUIKit.Core.Surface.ThemeStrokeWidth", DrUIKitTestFlags)
bool FDrUISurfaceStrokeTest::RunTest(const FString&) {
  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("테마 서브시스템"), Subsystem)) return false;
  UDrUITheme* Theme = UDrUITheme::MakeDefault(GetTransientPackage());
  Theme->ThemeId = TEXT("test-stroke");
  Subsystem->SetThemeObject(Theme);
  UDrSurface* Surface = NewObject<UDrSurface>();
  TestEqual(TEXT("테마 두께 0이면 부품 값(1px)"), Surface->ResolveStrokeWidth(), 1.f);
  Theme->StrokeWidth = 2.5f;
  TestEqual(TEXT("테마 기본 두께"), Surface->ResolveStrokeWidth(), 2.5f);
  Surface->StrokeWidth = 2.f;
  TestEqual(TEXT("강조(2px)는 1px 더"), Surface->ResolveStrokeWidth(), 3.5f);
  Surface->StrokeToken = NAME_None;
  TestEqual(TEXT("테두리 토큰이 없으면 0"), Surface->ResolveStrokeWidth(), 0.f);
  Subsystem->SetTheme(TEXT("default"));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGamepadInputTest, "DrUIKit.Gamepad.InputAndGlyphs", DrUIKitTestFlags)
bool FDrGamepadInputTest::RunTest(const FString&) {
  TestEqual(TEXT("A는 확인"), DrUIInput::GetActionForKey(EKeys::Gamepad_FaceButton_Bottom), EDrUIInputAction::Accept);
  TestEqual(TEXT("Esc는 뒤로"), DrUIInput::GetActionForKey(EKeys::Escape), EDrUIInputAction::Back);
  TestEqual(TEXT("RB는 다음 탭"), DrUIInput::GetActionForKey(EKeys::Gamepad_RightShoulder), EDrUIInputAction::NextTab);
  TestEqual(TEXT("LT는 이전 하위 탭"), DrUIInput::GetActionForKey(EKeys::Gamepad_LeftTrigger), EDrUIInputAction::PrevSubTab);
  TestEqual(TEXT("Q는 이전 탭(키보드)"), DrUIInput::GetActionForKey(EKeys::Q), EDrUIInputAction::PrevTab);
  TestEqual(TEXT("D패드는 뜻 없음(이동은 Slate)"), DrUIInput::GetActionForKey(EKeys::Gamepad_DPad_Up), EDrUIInputAction::None);
  auto Name = [](FKey Key, EDrUIGamepadGlyphs G) { return DrUIInput::GlyphName(Key, G).ToString(); };
  TestEqual(TEXT("Xbox 아래 = A"), Name(EKeys::Gamepad_FaceButton_Bottom, EDrUIGamepadGlyphs::Xbox), FString(TEXT("A")));
  TestEqual(TEXT("PlayStation 아래 = ✕"), Name(EKeys::Gamepad_FaceButton_Bottom, EDrUIGamepadGlyphs::PlayStation), FString(TEXT("✕")));
  TestEqual(TEXT("Nintendo 아래 = B(자리로 정함)"), Name(EKeys::Gamepad_FaceButton_Bottom, EDrUIGamepadGlyphs::Nintendo), FString(TEXT("B")));
  TestEqual(TEXT("PlayStation RB = R1"), Name(EKeys::Gamepad_RightShoulder, EDrUIGamepadGlyphs::PlayStation), FString(TEXT("R1")));
  TestEqual(TEXT("Nintendo LT = ZL"), Name(EKeys::Gamepad_LeftTrigger, EDrUIGamepadGlyphs::Nintendo), FString(TEXT("ZL")));
  TestEqual(TEXT("키보드 키는 패드 표기와 무관"), Name(EKeys::Escape, EDrUIGamepadGlyphs::PlayStation), FString(TEXT("Esc")));
  TestEqual(TEXT("PS5 패드 이름 → PlayStation"), DrUIInput::GlyphsForGamepadName(TEXT("PS5")), EDrUIGamepadGlyphs::PlayStation);
  TestEqual(TEXT("Switch 패드 이름 → Nintendo"), DrUIInput::GlyphsForGamepadName(TEXT("Switch")), EDrUIGamepadGlyphs::Nintendo);
  TestEqual(TEXT("모르는 이름 → Xbox"), DrUIInput::GlyphsForGamepadName(TEXT("Generic")), EDrUIGamepadGlyphs::Xbox);
  TestEqual(TEXT("얼굴 버튼 모양"), DrKeyGlyph::ShapeFor(EKeys::Gamepad_FaceButton_Top), EDrKeyShape::Face);
  TestEqual(TEXT("트리거 모양"), DrKeyGlyph::ShapeFor(EKeys::Gamepad_RightTrigger), EDrKeyShape::Trigger);
  TestEqual(TEXT("D패드 모양"), DrKeyGlyph::ShapeFor(EKeys::Gamepad_DPad_Left), EDrKeyShape::DPad);
  TestEqual(TEXT("스틱 기울기 모양"), DrKeyGlyph::ShapeFor(EKeys::Gamepad_Left2D), EDrKeyShape::StickAxis);
  TestEqual(TEXT("마우스 모양"), DrKeyGlyph::ShapeFor(EKeys::RightMouseButton), EDrKeyShape::Mouse);
  TestEqual(TEXT("키보드 키캡"), DrKeyGlyph::ShapeFor(EKeys::F1), EDrKeyShape::Keyboard);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGamepadFocusTest, "DrUIKit.Gamepad.FocusRingAndPrompt", DrUIKitTestFlags)
bool FDrGamepadFocusTest::RunTest(const FString&) {
  FDrUIStateInputs In;
  In.bFocused = true;
  TestTrue(TEXT("패드·키보드 포커스는 링"), DrUIState::IsFocusRingVisible(In));
  TestEqual(TEXT("패드·키보드 포커스는 Focus 상태"), DrUIState::Resolve(In), EDrUIState::Focus);
  In.bFocusVisible = false;
  TestFalse(TEXT("마우스로 생긴 포커스는 링 없음"), DrUIState::IsFocusRingVisible(In));
  TestEqual(TEXT("마우스로 생긴 포커스는 기본 모양"), DrUIState::Resolve(In), EDrUIState::Default);
  UDrButtonPrompt* Prompt = NewObject<UDrButtonPrompt>();
  Prompt->KeyboardKey = EKeys::Enter;
  Prompt->GamepadKey = EKeys::Gamepad_FaceButton_Bottom;
  bool bPad = false, bKey = false;
  Prompt->Device = EDrPromptDevice::Both;
  Prompt->GetShownKeys(bPad, bKey);
  TestTrue(TEXT("Both는 둘 다"), bPad && bKey);
  Prompt->Device = EDrPromptDevice::Keyboard;
  Prompt->GetShownKeys(bPad, bKey);
  TestTrue(TEXT("Keyboard는 키보드만"), !bPad && bKey);
  Prompt->GamepadKey = EKeys::Invalid;
  Prompt->Device = EDrPromptDevice::Gamepad;
  Prompt->GetShownKeys(bPad, bKey);
  TestTrue(TEXT("패드 키가 없으면 키보드로 대신"), !bPad && bKey);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGamepadInventoryTest, "DrUIKit.Gamepad.InventoryMath", DrUIKitTestFlags)
bool FDrGamepadInventoryTest::RunTest(const FString&) {
  // 6칸씩 3줄(18칸): 줄 끝에서 오른쪽은 다음 줄 처음, 줄 처음에서 왼쪽은 이전 줄 끝.
  TestEqual(TEXT("줄 안 오른쪽"), UDrItemGrid::WrapNeighbor(2, 1, 18, 6, true), 3);
  TestEqual(TEXT("줄 끝 오른쪽 → 다음 줄"), UDrItemGrid::WrapNeighbor(5, 1, 18, 6, true), 6);
  TestEqual(TEXT("줄 처음 왼쪽 → 이전 줄 끝"), UDrItemGrid::WrapNeighbor(6, -1, 18, 6, true), 5);
  TestEqual(TEXT("맨 끝 오른쪽은 없음"), UDrItemGrid::WrapNeighbor(17, 1, 18, 6, true), INDEX_NONE);
  TestEqual(TEXT("넘김 끄면 줄 끝에서 멈춤"), UDrItemGrid::WrapNeighbor(5, 1, 18, 6, false), INDEX_NONE);
  TestEqual(TEXT("마지막 줄이 짧으면 마지막 칸이 줄 끝"), UDrItemGrid::WrapNeighbor(13, 1, 14, 6, true), INDEX_NONE);
  FDrStatRow Row;
  Row.Current = 10.f; Row.New = 12.f;
  TestEqual(TEXT("높을수록 좋음: 오르면 +1"), UDrStatCompare::CompareDelta(Row), 1);
  Row.bHigherIsBetter = false;
  TestEqual(TEXT("낮을수록 좋음(무게): 오르면 -1"), UDrStatCompare::CompareDelta(Row), -1);
  Row.Current = 1.04f; Row.New = 1.01f; Row.Decimals = 1;
  TestEqual(TEXT("보이는 자릿수에서 같으면 0"), UDrStatCompare::CompareDelta(Row), 0);
  TestEqual(TEXT("희귀 → accent"), UDrItemSlot::RarityToken(EDrItemRarity::Rare), DrUITokens::Accent);
  for (float T : {0.f, .6f, 1.2f, 2.4f}) {
    const float O = UDrPressAnyKey::PulseOpacity(T);
    TestTrue(FString::Printf(TEXT("숨쉬기 밝기 %.1f초는 .35~1"), T), O >= .349f && O <= 1.001f);
  }
  TestEqual(TEXT("처음은 가장 밝게"), UDrPressAnyKey::PulseOpacity(0.f), 1.f);
  float Focus = 0.f, FocusTime = 0.f, Error = 0.f, ErrorTime = 0.f;
  DrUIFeedback::HapticFor(EDrUIFeedback::Focus, Focus, FocusTime);
  DrUIFeedback::HapticFor(EDrUIFeedback::Error, Error, ErrorTime);
  TestTrue(TEXT("오류 진동이 포커스 진동보다 세고 길다"), Error > Focus && ErrorTime > FocusTime);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGlyphImagesTest, "DrUIKit.Gamepad.GlyphImageSet", DrUIKitTestFlags)
bool FDrGlyphImagesTest::RunTest(const FString&) {
  // 작은 PNG를 써서 세트를 등록하고, 이미지가 실행 중에 텍스처로 읽히는지 본다.
  const FString Dir = FPaths::ProjectSavedDir() / TEXT("DrUIGlyphImageTest");
  IFileManager::Get().MakeDirectory(*Dir, true);
  TArray<FColor> Pixels;
  Pixels.Init(FColor::Red, 8 * 6);
  TArray64<uint8> Png;
  FImageUtils::PNGCompressImageArray(8, 6, Pixels, Png);
  TestTrue(TEXT("PNG 저장"), FFileHelper::SaveArrayToFile(Png, *(Dir / TEXT("a.png"))));
  // order를 크게: 실제 세트(Kenney 등)가 같은 키를 가지면 그쪽이 이긴다. 숫자패드 키는 Kenney 샘플 세트에 없는 키.
  const FString Json = TEXT(R"({"id":"test","name":"test","order":999,"license":"CC0","sets":{"keyboard":{"NumPadZero":"a.png","NotAKey":"a.png","NumPadOne":"missing.png"}}})");
  FDrUIGlyphImageSetInfo Info;
  FString Error;
  TestTrue(TEXT("등록"), DrUIGlyphImages::Register(Json, Dir, TEXT("Test"), Info, Error));
  TestEqual(TEXT("이미지 1개(모르는 키·없는 파일은 뺌)"), Info.ImageCount, 1);
  TestTrue(TEXT("빠진 것을 알림"), Error.Contains(TEXT("2")));
  TestEqual(TEXT("경로"), DrUIGlyphImages::FindPath(TEXT("keyboard"), EKeys::NumPadZero), Dir / TEXT("a.png"));
  if (UTexture2D* Texture = DrUIGlyphImages::Find(EDrUIGamepadGlyphs::PlayStation, EKeys::NumPadZero); TestNotNull(TEXT("키보드 키는 패드 표기와 무관하게 keyboard 세트"), Texture))
    TestEqual(TEXT("실행 중 텍스처 크기"), FIntPoint(Texture->GetSizeX(), Texture->GetSizeY()), FIntPoint(8, 6));
  TestNull(TEXT("없는 키는 벡터로(nullptr)"), DrUIGlyphImages::Find(EDrUIGamepadGlyphs::Xbox, EKeys::NumPadOne));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrStyleImagesTest, "DrUIKit.Style.Images", DrUIKitTestFlags)
bool FDrStyleImagesTest::RunTest(const FString&) {
  // style.json images: {mode}를 테마 id로 바꾸고, 없는 파일은 빼고 알리며, 이미지 토큰으로 칠하는 표면은 9분할·흐림 없음.
  const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("DrUIStyleImageTest"));
  IFileManager::Get().MakeDirectory(*(Dir / TEXT("Images")), true);
  TArray<FColor> Pixels;
  Pixels.Init(FColor::Orange, 64 * 32);
  TArray64<uint8> Png;
  FImageUtils::PNGCompressImageArray(64, 32, Pixels, Png);
  TestTrue(TEXT("PNG 저장"), FFileHelper::SaveArrayToFile(Png, *(Dir / TEXT("Images/panel-light.png"))));
  IFileManager::Get().Delete(*(Dir / TEXT("Images/panel-dark.png")));
  // MakeDefault는 고정 이름 객체라 둘을 만들면 같은 객체가 된다. 현재 테마를 복제해 쓴다.
  UDrUITheme* Light = DuplicateObject<UDrUITheme>(&UDrUIThemeSubsystem::Current(), GetTransientPackage());
  Light->ThemeId = TEXT("imgtest-light");
  Light->Images.Reset();
  UDrUITheme* Dark = DuplicateObject<UDrUITheme>(Light, GetTransientPackage());
  Dark->ThemeId = TEXT("imgtest-dark");
  TSharedPtr<FJsonObject> Style;
  FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(TEXT(R"({"images":{"glass":{"file":"Images/panel-{mode}.png","margin":[8,4,8,4],"scale":0.5,"tint":"#ff000080"}}})")), Style);
  FString Error;
  DrUIStyle::ApplyImages(Style, Dir, TEXT("imgtest"), {Light, Dark}, Error);
  TestTrue(TEXT("밝게에 이미지"), Light->Images.Contains(TEXT("glass")));
  TestFalse(TEXT("어둡게는 파일 없음 → 색으로"), Dark->Images.Contains(TEXT("glass")));
  TestTrue(TEXT("없는 파일을 알림"), Error.Contains(TEXT("panel-dark.png")));
  FSlateBrush Brush;
  if (TestTrue(TEXT("브러시"), Light->MakeImageBrush(TEXT("glass"), Brush))) {
    TestTrue(TEXT("9분할"), Brush.DrawAs == ESlateBrushDrawType::Box);
    TestTrue(TEXT("배율 반영 크기"), FVector2D(Brush.ImageSize).Equals(FVector2D(32, 16)));
    TestTrue(TEXT("여백은 이미지 크기 비율"), FMath::IsNearlyEqual(Brush.Margin.Left, 8.f / 64.f) && FMath::IsNearlyEqual(Brush.Margin.Top, 4.f / 32.f));
    TestTrue(TEXT("색 곱하기"), FMath::IsNearlyEqual(Brush.TintColor.GetSpecifiedColor().A, 128.f / 255.f, .01f));
  }
  TestFalse(TEXT("이미지 없는 토큰"), Light->MakeImageBrush(TEXT("glass-strong"), Brush));

  UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!TestNotNull(TEXT("서브시스템"), Subsystem)) return false;
  const FName Before = UDrUIThemeSubsystem::Current().ThemeId;
  Subsystem->AddTheme(Light);
  Subsystem->SetTheme(Light->ThemeId);
  UDrSurface* Surface = NewObject<UDrSurface>();
  TestTrue(TEXT("표면이 이미지로"), Surface->HasFillImage());
  TestEqual(TEXT("이미지 표면은 흐림 없음"), Surface->ResolveBlur(), 0.f);
  Subsystem->SetTheme(Before);
  return true;
}

#endif
