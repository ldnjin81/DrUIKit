#include "Widgets/DrButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "CommonInputSubsystem.h"
#include "Core/DrUIFeedback.h"
#include "Core/DrUIInput.h"
#include "Theme/DrUIThemeSubsystem.h"

namespace { TWeakObjectPtr<UDrButtonBase> GHintFocus; }

UDrButtonBase* UDrButtonBase::GetHintFocus() { return GHintFocus.Get(); }

UDrButtonStyle::UDrButtonStyle() {
  for (FSlateBrush* Brush : {&NormalBase, &NormalHovered, &NormalPressed, &SelectedBase, &SelectedHovered, &SelectedPressed, &Disabled})
    Brush->DrawAs = ESlateBrushDrawType::NoDrawType;
  ButtonPadding = FMargin(0.f);
  CustomPadding = FMargin(0.f);
}

UDrButtonBase::UDrButtonBase(const FObjectInitializer& Initializer) : Super(Initializer) {
  Style = UDrButtonStyle::StaticClass();
  Scale.Snap(1.f);
}

void UDrButtonBase::InitializeNativeClassData() {
  Super::InitializeNativeClassData();
  // CommonUI는 Initialize에서 그때의 루트를 내부 버튼으로 감싸고 누름·포커스를 잇는다. C++ 기본 구성은 그보다 먼저 있어야 한다
  // (RebuildWidget에서 만들면 내부 버튼 없이 그려져 마우스 누름이 닿지 않는다). 블루프린트 클래스는 여기를 지나지 않는다.
  if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
  if (!WidgetTree->RootWidget) BuildDefaultTree();
}

TSharedRef<SWidget> UDrButtonBase::RebuildWidget() {
  // 루트가 빈 위젯 블루프린트만 여기서 기본 구성을 만든다(내부 버튼으로 감싸지지 않아 마우스로는 눌리지 않는다).
  if (WidgetTree && !WidgetTree->RootWidget) BuildDefaultTree();
  // 만든 뒤에 정한 속성(라벨·변형 등)은 Initialize 때의 첫 칠하기에 없었다. 처음 그리기 전에 다시 칠한다.
  ApplyVisualState();
  return Super::RebuildWidget();
}

void UDrButtonBase::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyVisualState(); });
  RefreshState();
}

void UDrButtonBase::NativeConstruct() {
  Super::NativeConstruct();
  if (HasShortcut()) DrUIInput::RegisterShortcutButton(this);
  if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()); Input && !InputHandle.IsValid())
    InputHandle = Input->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
}

void UDrButtonBase::ResetInteractionState() {
  bFocusedNow = false;
  bPressedNow = false;
  if (GHintFocus == this) GHintFocus.Reset();
  RefreshState();
}

void UDrButtonBase::NativeDestruct() {
  bFocusedNow = false;
  bPressedNow = false;
  DrUIInput::UnregisterShortcutButton(this);
  if (GHintFocus == this) GHintFocus.Reset();
  if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer())) Input->OnInputMethodChangedNative.Remove(InputHandle);
  InputHandle.Reset();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

FDrUIStateInputs UDrButtonBase::GetStateInputs() const {
  FDrUIStateInputs In;
  In.bHovered = IsHovered();
  In.bPressed = bPressedNow;
  In.bFocused = bFocusedNow;
  // 마우스로 눌러 생긴 포커스는 숨긴다. 그 뒤 패드로 조작하기 시작하면 보인다.
  In.bFocusVisible = !bFocusFromPointer || DrUIInput::IsUsingGamepad(this);
  In.bDisabled = !GetIsEnabled();
  In.bSelected = GetSelected();
  return In;
}

bool UDrButtonBase::CanAnimate() const {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return bTicked && !(Subsystem && Subsystem->IsReduceMotion());
}

void UDrButtonBase::DriveSpring(FDrUISpring& Spring, float Target) const {
  if (CanAnimate()) Spring.Target = Target;
  else Spring.Snap(Target);
}

void UDrButtonBase::RefreshState() {
  VisualState = DrUIState::Resolve(GetStateInputs());
  const bool bReduce = UDrUIThemeSubsystem::Get() && UDrUIThemeSubsystem::Get()->IsReduceMotion();
  DriveSpring(Scale, VisualState == EDrUIState::Pressed && !bReduce ? PressedScale : 1.f);
  if (Scale.IsSettled()) SetRenderScale(FVector2D(Scale.Value));
  ApplyVisualState();
}

void UDrButtonBase::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  bTicked = true;
  if (!Scale.IsSettled()) {
    Scale.Step(DeltaTime);
    SetRenderScale(FVector2D(Scale.Value));
  }
}

void UDrButtonBase::NativeOnHovered() { Super::NativeOnHovered(); RefreshState(); }
void UDrButtonBase::NativeOnUnhovered() { Super::NativeOnUnhovered(); bPressedNow = false; RefreshState(); }
void UDrButtonBase::NativeOnPressed() { Super::NativeOnPressed(); bPressedNow = true; RefreshState(); }
void UDrButtonBase::NativeOnReleased() { Super::NativeOnReleased(); bPressedNow = false; RefreshState(); }
void UDrButtonBase::NativeOnSelected(bool bBroadcast) { Super::NativeOnSelected(bBroadcast); RefreshState(); }
void UDrButtonBase::NativeOnDeselected(bool bBroadcast) { Super::NativeOnDeselected(bBroadcast); RefreshState(); }
void UDrButtonBase::NativeOnEnabled() { Super::NativeOnEnabled(); RefreshState(); }
void UDrButtonBase::NativeOnDisabled() { Super::NativeOnDisabled(); RefreshState(); }
void UDrButtonBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) {
  Super::NativeOnAddedToFocusPath(InFocusEvent);
  bFocusedNow = true;
  bFocusFromPointer = InFocusEvent.GetCause() == EFocusCause::Mouse;
  GHintFocus = this;
  if (InFocusEvent.GetCause() == EFocusCause::Navigation) DrUIFeedback::Play(this, EDrUIFeedback::Focus);
  RefreshState();
}

void UDrButtonBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) {
  Super::NativeOnRemovedFromFocusPath(InFocusEvent);
  bFocusedNow = false;
  if (GHintFocus == this) GHintFocus.Reset();
  RefreshState();
}

void UDrButtonBase::NativeOnClicked() {
  Super::NativeOnClicked();
  DrUIFeedback::Play(this, EDrUIFeedback::Confirm);
}

void UDrButtonBase::HandleInputMethodChanged(ECommonInputType InputType) {
  // 마우스 포커스였던 버튼은 패드로 바꾸면 링이 보여야 한다.
  if (bFocusedNow) RefreshState();
}

void UDrButtonBase::SetShortcut(FKey InKeyboardKey, FKey InGamepadKey) {
  ShortcutKeyboardKey = InKeyboardKey;
  ShortcutGamepadKey = InGamepadKey;
  if (HasShortcut() && IsConstructed()) DrUIInput::RegisterShortcutButton(this);
  else if (!HasShortcut()) DrUIInput::UnregisterShortcutButton(this);
  OnShortcutChanged();
}

void UDrButtonBase::TriggerShortcut() {
  if (!GetIsEnabled() || !IsInteractionEnabled()) { DrUIFeedback::Play(this, EDrUIFeedback::Error); return; }
  // 누름 모양을 잠깐 보이고 클릭한다(CommonUI 클릭 경로: 선택·토글·OnClicked 모두).
  bPressedNow = true;
  RefreshState();
  HandleButtonClicked();
  bPressedNow = false;
  RefreshState();
}

void UDrButtonBase::TriggerHint(FName HintId) { OnHintAction.Broadcast(HintId); DrUIFeedback::Play(this, EDrUIFeedback::Confirm); }

FNavigationReply UDrButtonBase::NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) {
  FNavigationReply Reply = DefaultReply;
  if (NavTargets.Handle(this, Event.GetNavigationType(), Reply)) return Reply;
  return Super::NativeOnNavigation(Geometry, Event, DefaultReply);
}

FReply UDrButtonBase::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (!Event.IsRepeat())
    for (const FDrActionHint& Hint : ActionHints)
      if (Event.GetKey().IsValid() && (Event.GetKey() == Hint.GamepadKey || Event.GetKey() == Hint.KeyboardKey)) { TriggerHint(Hint.Id); return FReply::Handled(); }
  return Super::NativeOnKeyDown(Geometry, Event);
}
