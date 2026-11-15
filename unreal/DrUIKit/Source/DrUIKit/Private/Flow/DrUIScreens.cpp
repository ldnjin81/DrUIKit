#include "Flow/DrUIScreens.h"
#include "Core/DrUIFeedback.h"
#include "Flow/DrUIFlowSubsystem.h"
#include "InputMappingContext.h"
#include "Theme/DrUIKitSettings.h"

UDrUIMenuWidget::UDrUIMenuWidget(const FObjectInitializer& Initializer) : Super(Initializer) {
  bIsBackHandler = true;
  // 하위 메뉴에서 돌아오면 마지막으로 포커스했던 항목으로(패드로 처음부터 다시 내려오지 않게).
  bAutoRestoreFocus = true;
}

void UDrUIMenuWidget::NativeOnActivated() {
  InputMapping = GetDefault<UDrUIKitSettings>()->UIInputContext.LoadSynchronous();
  InputMappingPriority = 200;
  Super::NativeOnActivated();
}

TOptional<FUIInputConfig> UDrUIMenuWidget::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture); }

UWidget* UDrUIMenuWidget::NativeGetDesiredFocusTarget() const { return FirstFocus ? FirstFocus.Get() : Super::NativeGetDesiredFocusTarget(); }

bool UDrUIMenuWidget::NativeOnHandleBackAction() {
  DrUIFeedback::Play(this, EDrUIFeedback::Back);
  if (UDrUIFlowSubsystem* Flow = UDrUIFlowSubsystem::Get(this)) Flow->Back();
  return true;
}

FReply UDrUIMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_Special_Right) {
    if (!Event.IsRepeat()) NativeOnHandleBackAction();
    return FReply::Handled();
  }
  return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

TOptional<FUIInputConfig> UDrUIHUDWidget::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently); }
