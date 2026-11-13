#include "Flow/DrUIScreens.h"
#include "Flow/DrUIFlowSubsystem.h"
#include "InputMappingContext.h"
#include "Theme/DrUIKitSettings.h"

UDrUIMenuWidget::UDrUIMenuWidget(const FObjectInitializer& Initializer) : Super(Initializer) { bIsBackHandler = true; }

void UDrUIMenuWidget::NativeOnActivated() {
  InputMapping = GetDefault<UDrUIKitSettings>()->UIInputContext.LoadSynchronous();
  InputMappingPriority = 200;
  Super::NativeOnActivated();
}

TOptional<FUIInputConfig> UDrUIMenuWidget::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture); }

UWidget* UDrUIMenuWidget::NativeGetDesiredFocusTarget() const { return FirstFocus ? FirstFocus.Get() : Super::NativeGetDesiredFocusTarget(); }

bool UDrUIMenuWidget::NativeOnHandleBackAction() {
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
