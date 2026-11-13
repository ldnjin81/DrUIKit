#include "Widgets/DrExpandableArea.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

namespace {
bool IsExpanderReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
}

UDrExpanderHeader::UDrExpanderHeader(const FObjectInitializer& Initializer) : Super(Initializer) { PressedScale = 1.f; }

void UDrExpanderHeader::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusMd;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(12.f, 10.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(TitleText)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); }
  Chevron = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Chevron"));
  Chevron->Shape = EDrGlyphShape::ChevronDown;
  Chevron->GlyphSize = 16.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Chevron)) S->SetVerticalAlignment(VAlign_Center);
}

void UDrExpanderHeader::SetOpenAmount(float Amount) {
  // 닫히면 오른쪽(-90°), 열리면 아래(0°).
  if (Chevron) Chevron->SetRenderTransformAngle(-90.f * (1.f - Amount));
}

void UDrExpanderHeader::ApplyVisualState() {
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
  if (Surface) {
    const bool bLens = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
    Surface->FillToken = bLens ? DrUITokens::Hairline : NAME_None;
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  if (TitleText) { TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink); TitleText->SetText(Title); }
  if (Chevron) Chevron->SetColorToken(DrUITokens::InkMuted);
}

TSharedRef<SWidget> UDrExpandableArea::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    WidgetTree->RootWidget = Column;
    Header = WidgetTree->ConstructWidget<UDrExpanderHeader>(UDrExpanderHeader::StaticClass(), TEXT("Header"));
    Header->Title = Title;
    Column->AddChildToVerticalBox(Header);
    BodyBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BodyBox"));
    BodyBox->SetClipping(EWidgetClipping::ClipToBounds);  // 열리는 동안 넘치는 내용을 자른다
    Body = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Body"));
    Body->SetBrush(FSlateNoResource());
    Body->SetPadding(FMargin(12.f, 4.f, 12.f, 12.f));
    Body->SetVerticalAlignment(VAlign_Top);
    BodyBox->AddChild(Body);
    Column->AddChildToVerticalBox(BodyBox);
    Header->OnClicked().AddUObject(this, &ThisClass::Toggle);
    if (PendingBody) Body->SetContent(PendingBody);
    Open.Snap(bExpanded ? 1.f : 0.f);  // 처음은 제자리
    ApplyOpen();
  }
  return Super::RebuildWidget();
}

void UDrExpandableArea::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (Header && !Header->OnClicked().IsBoundToObject(this)) Header->OnClicked().AddUObject(this, &ThisClass::Toggle);  // 위젯 블루프린트가 둔 머리줄
}

void UDrExpandableArea::SetBody(UWidget* Content) {
  PendingBody = Content;
  if (Body) Body->SetContent(Content);
}

void UDrExpandableArea::SetExpanded(bool bInExpanded) {
  if (bExpanded == bInExpanded) return;
  bExpanded = bInExpanded;
  if (IsExpanderReduceMotion()) Open.Snap(bExpanded ? 1.f : 0.f);
  else Open.Target = bExpanded ? 1.f : 0.f;
  ApplyOpen();
  OnExpansionChanged.Broadcast(bExpanded);
}

void UDrExpandableArea::ApplyOpen() {
  if (!BodyBox) return;
  const float Amount = FMath::Clamp(Open.Value, 0.f, 1.f);
  if (Header) Header->SetOpenAmount(Amount);
  if (Amount <= 0.f && Open.IsSettled()) { BodyBox->SetVisibility(ESlateVisibility::Collapsed); return; }
  BodyBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  if (Amount >= 1.f && Open.IsSettled()) { BodyBox->ClearHeightOverride(); BodyBox->SetRenderOpacity(1.f); return; }
  // 내용 높이(보이는 동안 재어 둔 값)의 비율만큼만 보인다.
  const float Full = Body ? Body->GetDesiredSize().Y : 0.f;
  BodyBox->SetHeightOverride(Full * Amount);
  BodyBox->SetRenderOpacity(Amount);
}

void UDrExpandableArea::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Open.IsSettled()) {
    if (IsExpanderReduceMotion()) Open.Snap(Open.Target);
    else Open.Step(DeltaTime);
    ApplyOpen();
  }
}
