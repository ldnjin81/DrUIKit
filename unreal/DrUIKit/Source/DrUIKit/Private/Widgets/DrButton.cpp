#include "Widgets/DrButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

void UDrButton::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->RadiusToken = TEXT("radius-sm");
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(16.f, 10.f));
  Surface->BackdropToken = NAME_None;
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  Spinner = WidgetTree->ConstructWidget<UDrSpinner>(UDrSpinner::StaticClass(), TEXT("Spinner"));
  Spinner->SetRadius(7.f);
  Spinner->SetVisibility(ESlateVisibility::Collapsed);
  if (UHorizontalBoxSlot* SpinnerSlot = Row->AddChildToHorizontalBox(Spinner)) { SpinnerSlot->SetPadding(FMargin(0, 0, 6, 0)); SpinnerSlot->SetVerticalAlignment(VAlign_Center); }
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("label");
  if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText)) LabelSlot->SetVerticalAlignment(VAlign_Center);
}

void UDrButton::NativeOnInitialized() {
  ShownLabel = Label;
  Super::NativeOnInitialized();
}

FDrUIStateInputs UDrButton::GetStateInputs() const {
  FDrUIStateInputs In = Super::GetStateInputs();
  In.bLoading = bLoading;
  In.bSuccess = SuccessUntil >= 0.f;
  In.bError = bError;
  return In;
}

void UDrButton::ApplyVisualState() {
  if (!Surface || !LabelText) return;
  FName Fill = TEXT("accent"), Ink = TEXT("on-accent"), Stroke = NAME_None;
  switch (Variant) {
  case EDrButtonVariant::Secondary: Fill = TEXT("glass-strong"); Ink = TEXT("ink"); Stroke = TEXT("glass-stroke"); break;
  case EDrButtonVariant::Ghost: Fill = NAME_None; Ink = TEXT("ink"); break;
  case EDrButtonVariant::Destructive: Fill = TEXT("danger"); Ink = TEXT("on-accent"); break;
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
  if (Variant == EDrButtonVariant::Ghost && VisualState != EDrUIState::Error && VisualState != EDrUIState::Selected) Surface->SetBrushColor(FLinearColor::Transparent);
  LabelText->ColorToken = Ink;
  LabelText->ApplyTheme();
  LabelText->SetText(Text);
  if (Spinner) {
    Spinner->SetVisibility(VisualState == EDrUIState::Loading ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    Spinner->SetColorToken(Ink);  // 글자색과 같게
  }
}

void UDrButton::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (SuccessUntil >= 0.f) {
    SuccessUntil -= DeltaTime;
    if (SuccessUntil < 0.f) { SuccessUntil = -1.f; ShownLabel = Label; RefreshState(); }
  }
}

void UDrButton::SetLabel(const FText& InLabel) { Label = InLabel; ShownLabel = InLabel; ApplyVisualState(); }

void UDrButton::SetLoading(bool bInLoading) {
  bLoading = bInLoading;
  SetIsInteractionEnabled(!bInLoading);
  RefreshState();
}

void UDrButton::ShowSuccess(const FText& SuccessLabel) {
  bLoading = false; bError = false;
  SetIsInteractionEnabled(true);
  SuccessUntil = SuccessHoldSeconds;
  ShownLabel = SuccessLabel.IsEmpty() ? Label : SuccessLabel;
  RefreshState();
}

void UDrButton::SetError(bool bInError, const FText& ErrorLabel) {
  bError = bInError;
  ShownLabel = bInError && !ErrorLabel.IsEmpty() ? ErrorLabel : Label;
  RefreshState();
}
