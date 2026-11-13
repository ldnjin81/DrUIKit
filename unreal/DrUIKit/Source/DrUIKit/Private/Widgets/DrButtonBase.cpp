#include "Widgets/DrButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "Theme/DrUIThemeSubsystem.h"

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

TSharedRef<SWidget> UDrButtonBase::RebuildWidget() {
  // NativeOnInitialized의 첫 칠하기는 기본 구성이 생기기 전이라 비어 있다. 구성을 만든 뒤 한 번 더 칠한다.
  if (WidgetTree && !WidgetTree->RootWidget) { BuildDefaultTree(); ApplyVisualState(); }
  return Super::RebuildWidget();
}

void UDrButtonBase::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyVisualState(); });
  RefreshState();
}

void UDrButtonBase::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

FDrUIStateInputs UDrButtonBase::GetStateInputs() const {
  FDrUIStateInputs In;
  In.bHovered = IsHovered();
  In.bPressed = bPressedNow;
  In.bFocused = bFocusedNow;
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
void UDrButtonBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnAddedToFocusPath(InFocusEvent); bFocusedNow = true; RefreshState(); }
void UDrButtonBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnRemovedFromFocusPath(InFocusEvent); bFocusedNow = false; RefreshState(); }
