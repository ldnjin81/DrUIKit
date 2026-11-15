#include "Widgets/DrOptionRow.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/DrUIFeedback.h"
#include "Core/DrUIInput.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButtonPrompt.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

namespace { TWeakObjectPtr<UDrOptionRow> GFocusedRow; }

UDrOptionRow* UDrOptionRow::GetFocusedRow() { return GFocusedRow.Get(); }

TSharedRef<SWidget> UDrOptionRow::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
    Surface->FillStrongToken = NAME_None;
    Surface->BackdropToken = NAME_None;
    Surface->RadiusToken = DrUITokens::RadiusMd;
    Surface->PaddingToken = NAME_None;
    Surface->SetPadding(FMargin(14.f, 8.f));
    WidgetTree->RootWidget = Surface;
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Surface->SetContent(Row);
    LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetVerticalAlignment(VAlign_Center); S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
    ControlBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ControlBox"));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ControlBox)) S->SetVerticalAlignment(VAlign_Center);
    if (PendingControl) ControlBox->AddChild(PendingControl);
  }
  if (ActionHints.IsEmpty())
    ActionHints = {{TEXT("Adjust"), EKeys::Gamepad_LeftX, EKeys::Left, NSLOCTEXT("DrOptionRow", "Adjust", "조절")}};
  ApplyTheme();
  return Super::RebuildWidget();
}

void UDrOptionRow::SetLabel(const FText& InLabel) { Label = InLabel; ApplyTheme(); }

void UDrOptionRow::SetControl(UWidget* Control) {
  PendingControl = Control;
  if (ControlBox) { ControlBox->ClearChildren(); if (Control) ControlBox->AddChild(Control); }
}

void UDrOptionRow::NativeConstruct() {
  Super::NativeConstruct();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrOptionRow::NativeDestruct() {
  if (GFocusedRow == this) GFocusedRow.Reset();
  bFocusWithin = false;
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrOptionRow::ApplyTheme() {
  if (ControlBox) ControlBox->SetWidthOverride(ControlWidth);
  if (LabelText) { LabelText->SetTokens(DrUITokens::Body, DrUITokens::Ink); LabelText->SetText(Label); }
  if (!Surface) return;
  // 포커스가 줄 안이면 줄 전체 강조(패드·키보드면 포커스 테두리), 호버는 옅게.
  const bool bRing = bFocusWithin && DrUIInput::GetLastInputKind() != EDrUIInputKind::Pointer;
  Surface->FillToken = bFocusWithin ? DrUITokens::AccentSoft : bHoveredNow ? DrUITokens::Hairline : NAME_None;
  Surface->StrokeToken = bRing ? DrUITokens::Focus : NAME_None;
  Surface->StrokeWidth = 2.f;
  Surface->ApplyTheme();
}

void UDrOptionRow::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  const bool bWithin = HasFocusedDescendants(), bHover = IsHovered();
  // 마우스 ↔ 패드·키보드로 바뀌면 포커스 테두리도 바로 바뀐다.
  const bool bRing = bWithin && DrUIInput::GetLastInputKind() != EDrUIInputKind::Pointer;
  if (bWithin == bFocusWithin && bHover == bHoveredNow && bRing == bRingNow) return;
  bRingNow = bRing;
  const bool bEntered = bWithin && !bFocusWithin;
  bFocusWithin = bWithin;
  bHoveredNow = bHover;
  if (bWithin) GFocusedRow = this;
  else if (GFocusedRow == this) GFocusedRow.Reset();
  ApplyTheme();
  if (bEntered) OnFocusedNative.Broadcast();
}

FReply UDrOptionRow::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (Event.GetEffectingButton() == EKeys::LeftMouseButton && PendingControl && !PendingControl->HasAnyUserFocus() && !PendingControl->HasFocusedDescendants()) {
    DrUIInput::FocusWidget(PendingControl, true);
    return FReply::Handled();
  }
  return Super::NativeOnMouseButtonDown(Geometry, Event);
}

FNavigationReply UDrOptionRow::NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) {
  FNavigationReply Reply = DefaultReply;
  const EUINavigation Type = Event.GetNavigationType();
  if (NavTargets.Handle(this, Type, Reply)) return Reply;
  // 좌우는 조작 부품 몫: 부품이 쓰지 않고 올라온 좌우(끝에 닿음 등)는 줄 밖으로 내보내지 않는다(위에 덮개가 없을 때).
  if ((Type == EUINavigation::Left || Type == EUINavigation::Right) && DrUIInput::IsInActiveLayer(this)) return FNavigationReply::Stop();
  return Super::NativeOnNavigation(Geometry, Event, DefaultReply);
}
