#include "Widgets/DrButtonPrompt.h"
#include "Blueprint/WidgetTree.h"
#include "CommonInputSubsystem.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

FText UDrButtonPrompt::ShortKeyName(const FKey& Key) {
  static const TMap<FKey, const TCHAR*> Names = {
    {EKeys::Gamepad_FaceButton_Bottom, TEXT("A")}, {EKeys::Gamepad_FaceButton_Right, TEXT("B")},
    {EKeys::Gamepad_FaceButton_Left, TEXT("X")}, {EKeys::Gamepad_FaceButton_Top, TEXT("Y")},
    {EKeys::Gamepad_LeftShoulder, TEXT("LB")}, {EKeys::Gamepad_RightShoulder, TEXT("RB")},
    {EKeys::Gamepad_LeftTrigger, TEXT("LT")}, {EKeys::Gamepad_RightTrigger, TEXT("RT")},
    {EKeys::Gamepad_LeftThumbstick, TEXT("LS")}, {EKeys::Gamepad_RightThumbstick, TEXT("RS")},
    {EKeys::Gamepad_Special_Left, TEXT("View")}, {EKeys::Gamepad_Special_Right, TEXT("Menu")},
    {EKeys::Gamepad_DPad_Up, TEXT("↑")}, {EKeys::Gamepad_DPad_Down, TEXT("↓")}, {EKeys::Gamepad_DPad_Left, TEXT("←")}, {EKeys::Gamepad_DPad_Right, TEXT("→")},
    {EKeys::Escape, TEXT("Esc")}, {EKeys::Enter, TEXT("Enter")}, {EKeys::SpaceBar, TEXT("Space")}, {EKeys::Tab, TEXT("Tab")},
    {EKeys::BackSpace, TEXT("Back")}, {EKeys::LeftShift, TEXT("Shift")}, {EKeys::LeftControl, TEXT("Ctrl")}, {EKeys::LeftAlt, TEXT("Alt")},
    {EKeys::Up, TEXT("↑")}, {EKeys::Down, TEXT("↓")}, {EKeys::Left, TEXT("←")}, {EKeys::Right, TEXT("→")},
    {EKeys::LeftMouseButton, TEXT("LMB")}, {EKeys::RightMouseButton, TEXT("RMB")}, {EKeys::MiddleMouseButton, TEXT("MMB")}};
  if (const TCHAR* const* Found = Names.Find(Key)) return FText::FromString(*Found);
  return Key.IsValid() ? Key.GetDisplayName(false) : FText::GetEmpty();
}

bool UDrButtonPrompt::IsRoundKey(const FKey& Key) {
  return Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_FaceButton_Left || Key == EKeys::Gamepad_FaceButton_Top
    || Key == EKeys::Gamepad_LeftThumbstick || Key == EKeys::Gamepad_RightThumbstick;
}

FKey UDrButtonPrompt::GetShownKey() const {
  const bool bGamepad = Device == EDrPromptDevice::Gamepad || (Device == EDrPromptDevice::Auto && LastInputType == ECommonInputType::Gamepad);
  return bGamepad && GamepadKey.IsValid() ? GamepadKey : KeyboardKey.IsValid() ? KeyboardKey : GamepadKey;
}

void UDrButtonPrompt::SetKeys(FKey InKeyboardKey, FKey InGamepadKey, const FText& InLabel) {
  KeyboardKey = InKeyboardKey;
  GamepadKey = InGamepadKey;
  Label = InLabel;
  ApplyTheme();
}

void UDrButtonPrompt::SetDevice(EDrPromptDevice InDevice) { Device = InDevice; ApplyTheme(); }

void UDrButtonPrompt::SetHoldProgress(float Progress) {
  HoldProgress = FMath::Clamp(Progress, 0.f, 1.f);
  if (HoldBar) {
    HoldBar->SetPercent(HoldProgress);
    HoldBar->SetVisibility(HoldProgress > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);  // 자리는 남겨 줄이 흔들리지 않게
  }
}

TSharedRef<SWidget> UDrButtonPrompt::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
    WidgetTree->RootWidget = Row;
    UVerticalBox* CapColumn = WidgetTree->ConstructWidget<UVerticalBox>();
    CapBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CapBox"));
    CapBox->SetMinDesiredWidth(24.f);
    CapBox->SetHeightOverride(24.f);
    Cap = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Cap"));
    Cap->FillToken = DrUITokens::GlassStrong;
    Cap->BackdropToken = NAME_None;
    Cap->StrokeToken = DrUITokens::InkFaint;
    Cap->PaddingToken = NAME_None;
    Cap->SetPadding(FMargin(6.f, 0.f));
    Cap->SetHorizontalAlignment(HAlign_Center);
    Cap->SetVerticalAlignment(VAlign_Center);
    KeyText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("KeyText"));
    Cap->SetContent(KeyText);
    CapBox->AddChild(Cap);
    CapColumn->AddChildToVerticalBox(CapBox);
    HoldBar = WidgetTree->ConstructWidget<UDrProgressBar>(UDrProgressBar::StaticClass(), TEXT("HoldBar"));
    HoldBar->Thickness = 2.f;
    if (UVerticalBoxSlot* S = CapColumn->AddChildToVerticalBox(HoldBar)) S->SetPadding(FMargin(0, 3, 0, 0));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(CapColumn)) S->SetVerticalAlignment(VAlign_Center);
    LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0, 0, 0)); }
    ApplyTheme();
    SetHoldProgress(HoldProgress);
  }
  SetVisibility(ESlateVisibility::HitTestInvisible);
  return Super::RebuildWidget();
}

void UDrButtonPrompt::NativeConstruct() {
  Super::NativeConstruct();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()); Input && !InputHandle.IsValid()) {
    LastInputType = Input->GetCurrentInputType();
    InputHandle = Input->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
  }
  ApplyTheme();
}

void UDrButtonPrompt::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer())) Input->OnInputMethodChangedNative.Remove(InputHandle);
  ThemeHandle.Reset();
  InputHandle.Reset();
  Super::NativeDestruct();
}

void UDrButtonPrompt::HandleInputMethodChanged(ECommonInputType InputType) {
  LastInputType = InputType;
  ApplyTheme();
}

void UDrButtonPrompt::ApplyTheme() {
  const FKey Key = GetShownKey();
  const bool bRound = IsRoundKey(Key);
  if (Cap) {
    Cap->RadiusToken = bRound ? DrUITokens::RadiusPill : DrUITokens::RadiusXs;
    Cap->ApplyTheme();
  }
  if (CapBox) {  // 원은 정사각(24×24), 키캡은 글자만큼 넓어짐
    if (bRound) CapBox->SetMaxDesiredWidth(24.f);
    else CapBox->ClearMaxDesiredWidth();
  }
  if (KeyText) {
    KeyText->SetTokens(DrUITokens::Caption, DrUITokens::Ink);
    KeyText->SetText(ShortKeyName(Key));
  }
  if (LabelText) {
    LabelText->SetTokens(DrUITokens::Label, DrUITokens::InkMuted);
    LabelText->SetText(Label);
    LabelText->SetVisibility(Label.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
}
