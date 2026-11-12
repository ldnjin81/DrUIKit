#include "Widgets/DrUIButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "Theme/DrUIThemeSubsystem.h"

UDrUIButtonStyle::UDrUIButtonStyle() {
  for (FSlateBrush* Brush : {&NormalBase, &NormalHovered, &NormalPressed, &SelectedBase, &SelectedHovered, &SelectedPressed, &Disabled})
    Brush->DrawAs = ESlateBrushDrawType::NoDrawType;
  ButtonPadding = FMargin(0.f);
  CustomPadding = FMargin(0.f);
}

UDrUIButtonBase::UDrUIButtonBase(const FObjectInitializer& Initializer) : Super(Initializer) {
  Style = UDrUIButtonStyle::StaticClass();
  Scale.Snap(1.f);
}

TSharedRef<SWidget> UDrUIButtonBase::RebuildWidget() {
  // NativeOnInitialized의 첫 칠하기는 기본 구성이 생기기 전이라 비어 있다. 구성을 만든 뒤 한 번 더 칠한다.
  if (WidgetTree && !WidgetTree->RootWidget) { BuildDefaultTree(); ApplyVisualState(); }
  return Super::RebuildWidget();
}

void UDrUIButtonBase::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyVisualState(); });
  RefreshState();
}

void UDrUIButtonBase::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

FDrUIStateInputs UDrUIButtonBase::GetStateInputs() const {
  FDrUIStateInputs In;
  In.bHovered = IsHovered();
  In.bPressed = bPressedNow;
  In.bFocused = bFocusedNow;
  In.bDisabled = !GetIsEnabled();
  In.bSelected = GetSelected();
  return In;
}

bool UDrUIButtonBase::CanAnimate() const {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return bTicked && !(Subsystem && Subsystem->IsReduceMotion());
}

void UDrUIButtonBase::DriveSpring(FDrUISpring& Spring, float Target) const {
  if (CanAnimate()) Spring.Target = Target;
  else Spring.Snap(Target);
}

void UDrUIButtonBase::RefreshState() {
  VisualState = DrUIState::Resolve(GetStateInputs());
  const bool bReduce = UDrUIThemeSubsystem::Get() && UDrUIThemeSubsystem::Get()->IsReduceMotion();
  DriveSpring(Scale, VisualState == EDrUIState::Pressed && !bReduce ? PressedScale : 1.f);
  if (Scale.IsSettled()) SetRenderScale(FVector2D(Scale.Value));
  ApplyVisualState();
}

void UDrUIButtonBase::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  bTicked = true;
  if (!Scale.IsSettled()) {
    Scale.Step(DeltaTime);
    SetRenderScale(FVector2D(Scale.Value));
  }
}

void UDrUIButtonBase::NativeOnHovered() { Super::NativeOnHovered(); RefreshState(); }
void UDrUIButtonBase::NativeOnUnhovered() { Super::NativeOnUnhovered(); bPressedNow = false; RefreshState(); }
void UDrUIButtonBase::NativeOnPressed() { Super::NativeOnPressed(); bPressedNow = true; RefreshState(); }
void UDrUIButtonBase::NativeOnReleased() { Super::NativeOnReleased(); bPressedNow = false; RefreshState(); }
void UDrUIButtonBase::NativeOnSelected(bool bBroadcast) { Super::NativeOnSelected(bBroadcast); RefreshState(); }
void UDrUIButtonBase::NativeOnDeselected(bool bBroadcast) { Super::NativeOnDeselected(bBroadcast); RefreshState(); }
void UDrUIButtonBase::NativeOnEnabled() { Super::NativeOnEnabled(); RefreshState(); }
void UDrUIButtonBase::NativeOnDisabled() { Super::NativeOnDisabled(); RefreshState(); }
void UDrUIButtonBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnAddedToFocusPath(InFocusEvent); bFocusedNow = true; RefreshState(); }
void UDrUIButtonBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnRemovedFromFocusPath(InFocusEvent); bFocusedNow = false; RefreshState(); }
