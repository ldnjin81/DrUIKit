#include "Widgets/DrAnimatedSwitcher.h"
#include "Theme/DrUIThemeSubsystem.h"

UDrAnimatedSwitcher::UDrAnimatedSwitcher(const FObjectInitializer& Initializer) : Super(Initializer) {
  TransitionCurveType = ETransitionCurve::CubicOut;  // 디자인 시스템 이즈 아웃에 가장 가까운 엔진 곡선
  ApplyTransition();
}

void UDrAnimatedSwitcher::ApplyTransition() {
  switch (Transition) {
  case EDrTransition::Fade: TransitionType = ECommonSwitcherTransition::FadeOnly; break;
  case EDrTransition::SlideVertical: TransitionType = ECommonSwitcherTransition::Vertical; break;
  case EDrTransition::Zoom: TransitionType = ECommonSwitcherTransition::Zoom; break;
  default: TransitionType = ECommonSwitcherTransition::Horizontal; break;
  }
  TransitionDuration = Seconds;
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  SetDisableTransitionAnimation(Subsystem && Subsystem->IsReduceMotion());
}

TSharedRef<SWidget> UDrAnimatedSwitcher::RebuildWidget() {
  ApplyTransition();  // Slate 전환기는 만들 때 값을 받는다
  return Super::RebuildWidget();
}

void UDrAnimatedSwitcher::SetActiveWidgetIndex(int32 Index) {
  ApplyTransition();  // 움직임 줄이기는 실행 중에 바뀔 수 있다
  Super::SetActiveWidgetIndex(Index);
}

void UDrAnimatedSwitcher::SetActiveWidget(UWidget* Widget) {
  ApplyTransition();
  Super::SetActiveWidget(Widget);
}
