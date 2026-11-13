#include "Widgets/DrChip.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrChip::UDrChip(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = .96f;
  CheckIn.Snap(0.f);
}

void UDrChip::SetKind(EDrChipKind InKind) {
  Kind = InKind;
  SetIsSelectable(Kind != EDrChipKind::Input);
  SetIsToggleable(Kind == EDrChipKind::Filter);
  SetIsInteractableWhenSelected(Kind == EDrChipKind::Filter);
  ApplyVisualState();
}

void UDrChip::NativeOnInitialized() {
  SetKind(Kind);
  Super::NativeOnInitialized();
}

void UDrChip::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusPill;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(12.f, 6.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  Check = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Check"));
  Check->Shape = EDrGlyphShape::Check;
  Check->GlyphSize = 14.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Check)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 4, 0)); }
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) S->SetVerticalAlignment(VAlign_Center);
  Remove = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Remove"));
  Remove->Shape = EDrGlyphShape::Close;
  Remove->GlyphSize = 14.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Remove)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(6, 0, -4, 0)); }
}

void UDrChip::NativeOnClicked() {
  // 입력 칩: 오른쪽 × 칸(끝에서 28px)을 누르면 지우기.
  if (Kind == EDrChipKind::Input) {
    const FGeometry& Geometry = GetCachedGeometry();
    const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();  // Cursor는 UUserWidget 멤버라 이름을 피한다
    if (Geometry.IsUnderLocation(CursorPos) && Geometry.AbsoluteToLocal(CursorPos).X > Geometry.GetLocalSize().X - 28.f) { OnRemoved.Broadcast(this); return; }
  }
  Super::NativeOnClicked();
}

void UDrChip::ApplyVisualState() {
  const bool bOn = GetSelected();
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
  if (Surface) {
    const bool bLens = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
    Surface->FillToken = bOn ? DrUITokens::Chip : bLens ? DrUITokens::Hairline : DrUITokens::GlassStrong;
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : bOn ? NAME_None : DrUITokens::InkFaint;
    Surface->StrokeWidth = bFocusedNow ? 2.f : 1.f;
    Surface->ApplyTheme();
  }
  const FName Ink = bOn ? DrUITokens::OnChip : DrUITokens::Ink;
  if (LabelText) { LabelText->SetTokens(DrUITokens::Label, Ink); LabelText->SetText(Label); }
  DriveSpring(CheckIn, Kind == EDrChipKind::Filter && bOn ? 1.f : 0.f);
  if (Check) {
    Check->SetColorToken(Ink);
    Check->SetProgress(CheckIn.Value);
    Check->SetVisibility(CheckIn.Value > .01f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (Remove) {
    Remove->SetColorToken(bOn ? DrUITokens::OnChip : DrUITokens::InkMuted);
    Remove->SetVisibility(Kind == EDrChipKind::Input ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
}

void UDrChip::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!CheckIn.IsSettled()) {
    CheckIn.Step(DeltaTime);
    if (Check) {
      Check->SetProgress(CheckIn.Value);  // 체크를 그리며 칩이 늘어난다
      Check->SetVisibility(CheckIn.Value > .01f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
  }
}
