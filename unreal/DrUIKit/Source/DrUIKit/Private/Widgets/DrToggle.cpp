#include "Widgets/DrToggle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrToggleBase::UDrToggleBase(const FObjectInitializer& Initializer) : Super(Initializer) {
  bSelectable = true;
  bToggleable = true;
  bInteractableWhenSelected = true;
  PressedScale = 1.f;  // 줄 전체가 줄지 않게. 눌림은 표시 테두리로만 보인다.
}

void UDrToggleBase::SetLabel(const FText& InLabel) { Label = InLabel; ApplyVisualState(); }

void UDrToggleBase::BuildDefaultTree() {
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  WidgetTree->RootWidget = Row;
  if (UWidget* Indicator = BuildIndicator())
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Indicator)) S->SetVerticalAlignment(VAlign_Center);
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("body");
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0, 0, 0)); }
}

void UDrToggleBase::ApplyVisualState() {
  DriveSpring(On, IsOn() ? 1.f : 0.f);
  if (LabelText) {
    LabelText->SetVisibility(Label.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    LabelText->ColorToken = TEXT("ink");
    LabelText->ApplyTheme();
    LabelText->SetText(Label);
  }
  // 비활성은 모양을 지우지 않고 통째로 흐리게(옅은 색으로 칠하면 꺼진 상태가 바탕에 묻혔다).
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
  ApplyIndicator(On.Value);
}

void UDrToggleBase::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!On.IsSettled()) {
    On.Step(DeltaTime);
    ApplyIndicator(On.Value);
  }
}

void UDrToggleBase::PaintBox(UDrSurface* Box, float OnAmount) const {
  if (!Box) return;
  const bool bRing = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs());
  const bool bOn = OnAmount >= .5f;
  Box->FillToken = bOn ? TEXT("accent") : TEXT("glass-strong");
  Box->FillStrongToken = NAME_None;
  Box->StrokeToken = bRing ? FName(TEXT("focus")) : bOn ? NAME_None : VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed ? FName(TEXT("ink-muted")) : FName(TEXT("ink-faint"));
  Box->StrokeWidth = bRing ? 2.f : 1.5f;
  Box->ApplyTheme();
  Box->SetBrushColor(VisualState == EDrUIState::Pressed ? FLinearColor(.9f, .9f, .9f) : FLinearColor::White);
}

UDrCheckBox::UDrCheckBox(const FObjectInitializer& Initializer) : Super(Initializer) {}

UWidget* UDrCheckBox::BuildIndicator() {
  USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
  Size->SetWidthOverride(20.f);
  Size->SetHeightOverride(20.f);
  Box = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Box"));
  Box->RadiusToken = TEXT("radius-xs");
  Box->PaddingToken = NAME_None;
  Box->BackdropToken = NAME_None;
  Box->SetPadding(FMargin(0));
  Box->SetHorizontalAlignment(HAlign_Center);
  Box->SetVerticalAlignment(VAlign_Center);
  Mark = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Mark"));
  Mark->Shape = EDrGlyphShape::Check;
  Mark->GlyphSize = 16.f;
  Mark->Thickness = 2.2f;
  Box->SetContent(Mark);
  Size->AddChild(Box);
  return Size;
}

void UDrCheckBox::ApplyIndicator(float OnAmount) {
  PaintBox(Box, OnAmount);
  if (Mark) {
    Mark->ColorToken = TEXT("on-accent");
    Mark->Progress = OnAmount;
    Mark->ApplyTheme();
  }
}

UDrRadio::UDrRadio(const FObjectInitializer& Initializer) : Super(Initializer) { bToggleable = false; }

UWidget* UDrRadio::BuildIndicator() {
  UWidget* Indicator = Super::BuildIndicator();
  Box->RadiusToken = TEXT("radius-pill");
  Mark->Shape = EDrGlyphShape::Dot;
  return Indicator;
}

UDrSwitch::UDrSwitch(const FObjectInitializer& Initializer) : Super(Initializer) {}

UWidget* UDrSwitch::BuildIndicator() {
  USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
  Size->SetWidthOverride(DrSwitchSize::TrackWidth);
  Size->SetHeightOverride(DrSwitchSize::TrackHeight);
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>();
  Size->AddChild(Layers);
  Track = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Track"));
  Track->RadiusToken = TEXT("radius-pill");
  Track->PaddingToken = NAME_None;
  Track->BackdropToken = NAME_None;
  if (UOverlaySlot* S = Layers->AddChildToOverlay(Track)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  USizeBox* ThumbSizeBox = WidgetTree->ConstructWidget<USizeBox>();
  ThumbSizeBox->SetWidthOverride(DrSwitchSize::ThumbSize);
  ThumbSizeBox->SetHeightOverride(DrSwitchSize::ThumbSize);
  Thumb = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Thumb"));
  Thumb->RadiusToken = TEXT("radius-pill");
  Thumb->PaddingToken = NAME_None;
  Thumb->BackdropToken = NAME_None;
  Thumb->StrokeToken = NAME_None;
  ThumbSizeBox->AddChild(Thumb);
  if (UOverlaySlot* S = Layers->AddChildToOverlay(ThumbSizeBox)) { S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(DrSwitchSize::Inset, 0, 0, 0)); }
  return Size;
}

void UDrSwitch::ApplyIndicator(float OnAmount) {
  const bool bRing = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs());
  if (Track) {
    // 트랙 색은 손잡이가 가운데를 지날 때 바뀐다.
    Track->FillToken = OnAmount >= .5f ? TEXT("accent") : TEXT("ink-faint");
    Track->FillStrongToken = NAME_None;
    Track->StrokeToken = bRing ? FName(TEXT("focus")) : NAME_None;
    Track->StrokeWidth = 2.f;
    Track->ApplyTheme();
    Track->SetBrushColor(VisualState == EDrUIState::Hover ? FLinearColor(1.08f, 1.08f, 1.08f) : VisualState == EDrUIState::Pressed ? FLinearColor(.92f, .92f, .92f) : FLinearColor::White);
  }
  if (Thumb) {
    Thumb->FillToken = TEXT("on-accent");
    Thumb->FillStrongToken = NAME_None;
    Thumb->ApplyTheme();
    Thumb->SetRenderTranslation(FVector2D(ThumbTravel(OnAmount), 0.f));
  }
}
