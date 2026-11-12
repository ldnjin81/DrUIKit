#include "Widgets/DrUIButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CircularThrobber.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"

UDrUIButtonStyle::UDrUIButtonStyle() {
  for (FSlateBrush* Brush : {&NormalBase, &NormalHovered, &NormalPressed, &SelectedBase, &SelectedHovered, &SelectedPressed, &Disabled})
    Brush->DrawAs = ESlateBrushDrawType::NoDrawType;
  ButtonPadding = FMargin(0.f);
  CustomPadding = FMargin(0.f);
}

UDrUIButton::UDrUIButton(const FObjectInitializer& Initializer) : Super(Initializer) {
  Style = UDrUIButtonStyle::StaticClass();
  Scale.Snap(1.f);
}

void UDrUIButton::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Surface"));
  Surface->RadiusToken = TEXT("radius-sm");
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(16.f, 10.f));
  Surface->BackdropToken = NAME_None;
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  Spinner = WidgetTree->ConstructWidget<UCircularThrobber>(UCircularThrobber::StaticClass(), TEXT("Spinner"));
  Spinner->SetNumberOfPieces(6);
  Spinner->SetRadius(7.f);
  Spinner->SetVisibility(ESlateVisibility::Collapsed);
  if (UHorizontalBoxSlot* SpinnerSlot = Row->AddChildToHorizontalBox(Spinner)) { SpinnerSlot->SetPadding(FMargin(0, 0, 6, 0)); SpinnerSlot->SetVerticalAlignment(VAlign_Center); }
  LabelText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("label");
  if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText)) LabelSlot->SetVerticalAlignment(VAlign_Center);
}

TSharedRef<SWidget> UDrUIButton::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) BuildDefaultTree();
  return Super::RebuildWidget();
}

void UDrUIButton::NativeOnInitialized() {
  Super::NativeOnInitialized();
  ShownLabel = Label;
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyVisualState(); });
  RefreshState();
}

void UDrUIButton::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

FDrUIStateInputs UDrUIButton::GetStateInputs() const {
  FDrUIStateInputs In;
  In.bHovered = IsHovered();
  In.bPressed = bPressedNow;
  In.bFocused = bFocusedNow;
  In.bDisabled = !GetIsEnabled();
  In.bLoading = bLoading;
  In.bSuccess = SuccessUntil >= 0.f;
  In.bError = bError;
  In.bSelected = GetSelected();
  return In;
}

void UDrUIButton::RefreshState() {
  const EDrUIState Next = DrUIState::Resolve(GetStateInputs());
  const bool bReduce = UDrUIThemeSubsystem::Get() && UDrUIThemeSubsystem::Get()->IsReduceMotion();
  Scale.Target = Next == EDrUIState::Pressed && !bReduce ? .97f : 1.f;
  VisualState = Next;
  ApplyVisualState();
}

void UDrUIButton::ApplyVisualState() {
  if (!Surface || !LabelText) return;
  FName Fill = TEXT("accent"), Ink = TEXT("on-accent"), Stroke = NAME_None;
  switch (Variant) {
  case EDrUIButtonVariant::Secondary: Fill = TEXT("glass-strong"); Ink = TEXT("ink"); Stroke = TEXT("glass-stroke"); break;
  case EDrUIButtonVariant::Ghost: Fill = NAME_None; Ink = TEXT("ink"); break;
  case EDrUIButtonVariant::Destructive: Fill = TEXT("danger"); Ink = TEXT("on-accent"); break;
  default: break;
  }
  FText Text = ShownLabel.IsEmpty() ? Label : ShownLabel;
  switch (VisualState) {
  case EDrUIState::Disabled: Fill = TEXT("hairline"); Ink = TEXT("ink-faint"); Stroke = NAME_None; break;
  case EDrUIState::Success: Fill = TEXT("success"); Ink = TEXT("on-accent"); break;
  case EDrUIState::Error: Fill = TEXT("glass-strong"); Ink = TEXT("danger"); Stroke = TEXT("danger"); break;
  case EDrUIState::Selected: Fill = TEXT("glass-strong"); Ink = TEXT("ink"); Stroke = TEXT("ink"); break;
  default: break;
  }
  Surface->FillToken = Fill.IsNone() ? TEXT("glass") : Fill;
  Surface->FillStrongToken = NAME_None;
  Surface->StrokeToken = Stroke;
  Surface->StrokeWidth = VisualState == EDrUIState::Focus ? 2.f : 1.f;
  if (VisualState == EDrUIState::Focus) Surface->StrokeToken = TEXT("focus");
  Surface->ApplyTheme();
  // 호버는 살짝 밝게, 눌림은 살짝 어둡게(채움 색은 같은 토큰).
  Surface->SetContentColorAndOpacity(FLinearColor::White);
  Surface->SetBrushColor(VisualState == EDrUIState::Hover ? FLinearColor(1.08f, 1.08f, 1.08f) : VisualState == EDrUIState::Pressed ? FLinearColor(.92f, .92f, .92f) : FLinearColor::White);
  if (Variant == EDrUIButtonVariant::Ghost && VisualState != EDrUIState::Error && VisualState != EDrUIState::Selected) Surface->SetBrushColor(FLinearColor::Transparent);
  LabelText->ColorToken = Ink;
  LabelText->ApplyTheme();
  LabelText->SetText(Text);
  if (Spinner) {
    Spinner->SetVisibility(VisualState == EDrUIState::Loading ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    // 스피너 조각은 기본 이미지를 쓰므로 글자색으로 칠한다.
    FSlateBrush Piece = Spinner->GetImage();
    Piece.TintColor = FSlateColor(UDrUIThemeSubsystem::Current().GetColor(Ink));
    Spinner->SetImage(Piece);
  }
}

void UDrUIButton::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (SuccessUntil >= 0.f) {
    SuccessUntil -= DeltaTime;
    if (SuccessUntil < 0.f) { SuccessUntil = -1.f; ShownLabel = Label; RefreshState(); }
  }
  if (!Scale.IsSettled()) {
    Scale.Step(DeltaTime);
    SetRenderScale(FVector2D(Scale.Value));
  }
}

void UDrUIButton::SetLabel(const FText& InLabel) { Label = InLabel; ShownLabel = InLabel; ApplyVisualState(); }

void UDrUIButton::SetLoading(bool bInLoading) {
  bLoading = bInLoading;
  SetIsInteractionEnabled(!bInLoading);
  RefreshState();
}

void UDrUIButton::ShowSuccess(const FText& SuccessLabel) {
  bLoading = false; bError = false;
  SetIsInteractionEnabled(true);
  SuccessUntil = SuccessHoldSeconds;
  ShownLabel = SuccessLabel.IsEmpty() ? Label : SuccessLabel;
  RefreshState();
}

void UDrUIButton::SetError(bool bInError, const FText& ErrorLabel) {
  bError = bInError;
  ShownLabel = bInError && !ErrorLabel.IsEmpty() ? ErrorLabel : Label;
  RefreshState();
}

void UDrUIButton::NativeOnHovered() { Super::NativeOnHovered(); RefreshState(); }
void UDrUIButton::NativeOnUnhovered() { Super::NativeOnUnhovered(); bPressedNow = false; RefreshState(); }
void UDrUIButton::NativeOnPressed() { Super::NativeOnPressed(); bPressedNow = true; RefreshState(); }
void UDrUIButton::NativeOnReleased() { Super::NativeOnReleased(); bPressedNow = false; RefreshState(); }
void UDrUIButton::NativeOnSelected(bool bBroadcast) { Super::NativeOnSelected(bBroadcast); RefreshState(); }
void UDrUIButton::NativeOnDeselected(bool bBroadcast) { Super::NativeOnDeselected(bBroadcast); RefreshState(); }
void UDrUIButton::NativeOnEnabled() { Super::NativeOnEnabled(); RefreshState(); }
void UDrUIButton::NativeOnDisabled() { Super::NativeOnDisabled(); RefreshState(); }
void UDrUIButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnAddedToFocusPath(InFocusEvent); bFocusedNow = true; RefreshState(); }
void UDrUIButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) { Super::NativeOnRemovedFromFocusPath(InFocusEvent); bFocusedNow = false; RefreshState(); }
