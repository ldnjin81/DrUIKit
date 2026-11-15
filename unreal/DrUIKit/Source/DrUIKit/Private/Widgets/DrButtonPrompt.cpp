#include "Widgets/DrButtonPrompt.h"
#include "Blueprint/WidgetTree.h"
#include "CommonInputSubsystem.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrKeyGlyph.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrText.h"

FText UDrButtonPrompt::ShortKeyName(const FKey& Key) { return DrUIInput::GlyphName(Key, EDrGamepadGlyphs::Xbox); }

bool UDrButtonPrompt::IsRoundKey(const FKey& Key) {
  const EDrKeyShape Shape = DrKeyGlyph::ShapeFor(Key);
  return Shape == EDrKeyShape::Face || Shape == EDrKeyShape::Stick || Shape == EDrKeyShape::StickAxis;
}

void UDrButtonPrompt::GetShownKeys(bool& bOutGamepad, bool& bOutKeyboard) const {
  const bool bHasPad = GamepadKey.IsValid(), bHasKey = KeyboardKey.IsValid();
  switch (Device) {
    case EDrPromptDevice::Both: bOutGamepad = bHasPad; bOutKeyboard = bHasKey; break;
    case EDrPromptDevice::Gamepad: bOutGamepad = bHasPad; bOutKeyboard = !bHasPad && bHasKey; break;
    case EDrPromptDevice::Keyboard: bOutKeyboard = bHasKey; bOutGamepad = !bHasKey && bHasPad; break;
    default: {
      const bool bPad = LastInputType == ECommonInputType::Gamepad;
      bOutGamepad = bHasPad && (bPad || !bHasKey);
      bOutKeyboard = bHasKey && !bOutGamepad;
    }
  }
}

FKey UDrButtonPrompt::GetShownKey() const {
  bool bPad = false, bKey = false;
  GetShownKeys(bPad, bKey);
  return bPad ? GamepadKey : KeyboardKey;
}

void UDrButtonPrompt::SetKeys(FKey InKeyboardKey, FKey InGamepadKey, const FText& InLabel) {
  KeyboardKey = InKeyboardKey;
  GamepadKey = InGamepadKey;
  Label = InLabel;
  ApplyTheme();
}

void UDrButtonPrompt::SetDevice(EDrPromptDevice InDevice) { Device = InDevice; ApplyTheme(); }
void UDrButtonPrompt::SetGlyphs(EDrGamepadGlyphs InGlyphs) { Glyphs = InGlyphs; ApplyTheme(); }

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
    // 키(패드 · 구분 · 키보드) 아래에 길게 누르기 막대.
    UVerticalBox* KeyColumn = WidgetTree->ConstructWidget<UVerticalBox>();
    UHorizontalBox* Keys = WidgetTree->ConstructWidget<UHorizontalBox>();
    GamepadGlyph = WidgetTree->ConstructWidget<UDrKeyGlyph>(UDrKeyGlyph::StaticClass(), TEXT("GamepadGlyph"));
    if (UHorizontalBoxSlot* S = Keys->AddChildToHorizontalBox(GamepadGlyph)) S->SetVerticalAlignment(VAlign_Center);
    Separator = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("Separator"));
    Separator->SetText(FText::FromString(TEXT("/")));
    if (UHorizontalBoxSlot* S = Keys->AddChildToHorizontalBox(Separator)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(4, 0)); }
    KeyboardGlyph = WidgetTree->ConstructWidget<UDrKeyGlyph>(UDrKeyGlyph::StaticClass(), TEXT("KeyboardGlyph"));
    if (UHorizontalBoxSlot* S = Keys->AddChildToHorizontalBox(KeyboardGlyph)) S->SetVerticalAlignment(VAlign_Center);
    KeyColumn->AddChildToVerticalBox(Keys);
    HoldBar = WidgetTree->ConstructWidget<UDrProgressBar>(UDrProgressBar::StaticClass(), TEXT("HoldBar"));
    HoldBar->Thickness = 2.f;
    if (UVerticalBoxSlot* S = KeyColumn->AddChildToVerticalBox(HoldBar)) S->SetPadding(FMargin(0, 3, 0, 0));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(KeyColumn)) S->SetVerticalAlignment(VAlign_Center);
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
  bool bPad = false, bKey = false;
  GetShownKeys(bPad, bKey);
  for (UDrKeyGlyph* Glyph : {GamepadGlyph.Get(), KeyboardGlyph.Get()}) {
    if (!Glyph) continue;
    Glyph->Key = Glyph == GamepadGlyph ? GamepadKey : KeyboardKey;
    Glyph->Glyphs = Glyphs;
    Glyph->Height = KeyHeight;
    Glyph->bBrandColors = bBrandColors;
    Glyph->ApplyTheme();
    Glyph->SetVisibility((Glyph == GamepadGlyph ? bPad : bKey) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (Separator) {
    Separator->SetTokens(DrUITokens::Caption, DrUITokens::InkFaint);
    Separator->SetVisibility(bPad && bKey ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (LabelText) {
    LabelText->SetTokens(DrUITokens::Label, DrUITokens::InkMuted);
    LabelText->SetText(Label);
    LabelText->SetVisibility(Label.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
}
