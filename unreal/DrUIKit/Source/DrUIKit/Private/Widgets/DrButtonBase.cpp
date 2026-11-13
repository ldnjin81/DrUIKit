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
