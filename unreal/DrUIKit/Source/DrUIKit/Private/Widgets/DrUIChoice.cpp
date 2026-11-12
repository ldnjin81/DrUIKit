#include "Widgets/DrUIChoice.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Widgets/DrUIGlyph.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"

UDrUIChoiceBase::UDrUIChoiceBase(const FObjectInitializer& Initializer) : Super(Initializer) {
  bSelectable = true;
  bToggleable = true;
  bInteractableWhenSelected = true;
  PressedScale = 1.f;  // 줄 전체가 줄지 않게. 눌림은 표시 테두리로만 보인다.
}

void UDrUIChoiceBase::SetLabel(const FText& InLabel) { Label = InLabel; ApplyVisualState(); }

void UDrUIChoiceBase::BuildDefaultTree() {
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  WidgetTree->RootWidget = Row;
  if (UWidget* Indicator = BuildIndicator())
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Indicator)) S->SetVerticalAlignment(VAlign_Center);
  LabelText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("body");
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0, 0, 0)); }
}

void UDrUIChoiceBase::ApplyVisualState() {
  DriveSpring(On, IsOn() ? 1.f : 0.f);
  if (LabelText) {
    LabelText->SetVisibility(Label.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    LabelText->ColorToken = VisualState == EDrUIState::Disabled ? TEXT("ink-faint") : TEXT("ink");
    LabelText->ApplyTheme();
    LabelText->SetText(Label);
  }
  ApplyIndicator(On.Value);
}

void UDrUIChoiceBase::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!On.IsSettled()) {
    On.Step(DeltaTime);
    ApplyIndicator(On.Value);
  }
}

void UDrUIChoiceBase::PaintBox(UDrUISurface* Box, float OnAmount) const {
  if (!Box) return;
  const bool bDisabled = VisualState == EDrUIState::Disabled;
  const bool bRing = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs());
  const bool bOn = OnAmount >= .5f;
  Box->FillToken = bDisabled ? TEXT("hairline") : bOn ? TEXT("accent") : TEXT("glass-strong");
  Box->FillStrongToken = NAME_None;
  Box->StrokeToken = bRing ? FName(TEXT("focus")) : (bOn || bDisabled) ? NAME_None : VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed ? FName(TEXT("ink-muted")) : FName(TEXT("ink-faint"));
  Box->StrokeWidth = bRing ? 2.f : 1.5f;
  Box->ApplyTheme();
  Box->SetBrushColor(VisualState == EDrUIState::Pressed ? FLinearColor(.9f, .9f, .9f) : FLinearColor::White);
}

UDrUICheckBox::UDrUICheckBox(const FObjectInitializer& Initializer) : Super(Initializer) {}

UWidget* UDrUICheckBox::BuildIndicator() {
  USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
  Size->SetWidthOverride(20.f);
  Size->SetHeightOverride(20.f);
  Box = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Box"));
  Box->RadiusToken = TEXT("radius-xs");
  Box->PaddingToken = NAME_None;
  Box->BackdropToken = NAME_None;
  Box->SetPadding(FMargin(0));
  Box->SetHorizontalAlignment(HAlign_Center);
  Box->SetVerticalAlignment(VAlign_Center);
  Mark = WidgetTree->ConstructWidget<UDrUIGlyph>(UDrUIGlyph::StaticClass(), TEXT("Mark"));
  Mark->Shape = EDrUIGlyphShape::Check;
  Mark->GlyphSize = 16.f;
  Mark->Thickness = 2.2f;
  Box->SetContent(Mark);
  Size->AddChild(Box);
  return Size;
}

void UDrUICheckBox::ApplyIndicator(float OnAmount) {
  PaintBox(Box, OnAmount);
  if (Mark) {
    Mark->ColorToken = VisualState == EDrUIState::Disabled ? TEXT("ink-faint") : TEXT("on-accent");
    Mark->Progress = OnAmount;
    Mark->ApplyTheme();
  }
}

UDrUIRadio::UDrUIRadio(const FObjectInitializer& Initializer) : Super(Initializer) { bToggleable = false; }

UWidget* UDrUIRadio::BuildIndicator() {
  UWidget* Indicator = Super::BuildIndicator();
  Box->RadiusToken = TEXT("radius-pill");
  Mark->Shape = EDrUIGlyphShape::Dot;
  return Indicator;
}

UDrUISwitch::UDrUISwitch(const FObjectInitializer& Initializer) : Super(Initializer) {}

UWidget* UDrUISwitch::BuildIndicator() {
  USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
  Size->SetWidthOverride(DrUISwitchSize::TrackWidth);
  Size->SetHeightOverride(DrUISwitchSize::TrackHeight);
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>();
  Size->AddChild(Layers);
  Track = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Track"));
  Track->RadiusToken = TEXT("radius-pill");
  Track->PaddingToken = NAME_None;
  Track->BackdropToken = NAME_None;
  if (UOverlaySlot* S = Layers->AddChildToOverlay(Track)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  USizeBox* ThumbSizeBox = WidgetTree->ConstructWidget<USizeBox>();
  ThumbSizeBox->SetWidthOverride(DrUISwitchSize::ThumbSize);
  ThumbSizeBox->SetHeightOverride(DrUISwitchSize::ThumbSize);
  Thumb = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Thumb"));
  Thumb->RadiusToken = TEXT("radius-pill");
  Thumb->PaddingToken = NAME_None;
  Thumb->BackdropToken = NAME_None;
  Thumb->StrokeToken = NAME_None;
  ThumbSizeBox->AddChild(Thumb);
  if (UOverlaySlot* S = Layers->AddChildToOverlay(ThumbSizeBox)) { S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(DrUISwitchSize::Inset, 0, 0, 0)); }
  return Size;
}

void UDrUISwitch::ApplyIndicator(float OnAmount) {
  const bool bDisabled = VisualState == EDrUIState::Disabled;
  const bool bRing = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs());
  if (Track) {
    // 트랙 색은 손잡이가 가운데를 지날 때 바뀐다.
    Track->FillToken = bDisabled ? TEXT("hairline") : OnAmount >= .5f ? TEXT("accent") : TEXT("ink-faint");
    Track->FillStrongToken = NAME_None;
    Track->StrokeToken = bRing ? FName(TEXT("focus")) : NAME_None;
    Track->StrokeWidth = 2.f;
    Track->ApplyTheme();
    Track->SetBrushColor(VisualState == EDrUIState::Hover ? FLinearColor(1.08f, 1.08f, 1.08f) : VisualState == EDrUIState::Pressed ? FLinearColor(.92f, .92f, .92f) : FLinearColor::White);
  }
  if (Thumb) {
    Thumb->FillToken = bDisabled ? TEXT("glass-strong") : TEXT("on-accent");
    Thumb->FillStrongToken = NAME_None;
    Thumb->ApplyTheme();
    Thumb->SetRenderTranslation(FVector2D(ThumbTravel(OnAmount), 0.f));
  }
}
