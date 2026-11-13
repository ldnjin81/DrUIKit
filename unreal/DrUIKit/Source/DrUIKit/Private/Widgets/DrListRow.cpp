#include "Widgets/DrListRow.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrListItem* UDrListItem::Make(UObject* Outer, const FText& InTitle, const FText& InSubtitle, const FText& InTrailing) {
  UDrListItem* Item = NewObject<UDrListItem>(Outer ? Outer : GetTransientPackage());
  Item->Title = InTitle;
  Item->Subtitle = InSubtitle;
  Item->Trailing = InTrailing;
  return Item;
}

UDrListRow::UDrListRow(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = 1.f;  // 목록 행은 줄지 않는다(옆 행과 어긋나 보임)
}

void UDrListRow::SetContent(const FText& InTitle, const FText& InSubtitle, const FText& InTrailing) {
  Title = InTitle;
  Subtitle = InSubtitle;
  Trailing = InTrailing;
  ApplyVisualState();
}

void UDrListRow::NativeOnListItemObjectSet(UObject* ListItemObject) {
  IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
  if (const UDrListItem* Item = Cast<UDrListItem>(ListItemObject)) SetContent(Item->Title, Item->Subtitle, Item->Trailing);
}

void UDrListRow::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusMd;  // 디자인 시스템: radius-md = 목록 행 호버 렌즈
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(12.f, 9.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  UVerticalBox* Texts = WidgetTree->ConstructWidget<UVerticalBox>();
  TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
  Texts->AddChildToVerticalBox(TitleText);
  SubtitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("SubtitleText"));
  Texts->AddChildToVerticalBox(SubtitleText);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Texts)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); }
  TrailingText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TrailingText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(TrailingText)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 0, 0)); }
  Chevron = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Chevron"));
  Chevron->Shape = EDrGlyphShape::ChevronRight;
  Chevron->GlyphSize = 16.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Chevron)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0, 0, 0)); }
}

void UDrListRow::ApplyVisualState() {
  const bool bDisabled = VisualState == EDrUIState::Disabled;
  SetRenderOpacity(bDisabled ? .4f : 1.f);
  if (Surface) {
    const bool bLens = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
    Surface->FillToken = GetSelected() ? DrUITokens::AccentSoft : bLens ? DrUITokens::Hairline : NAME_None;
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  if (TitleText) {
    TitleText->SetTokens(DrUITokens::Body, DrUITokens::Ink);
    TitleText->SetText(Title);
  }
  if (SubtitleText) {
    SubtitleText->SetTokens(DrUITokens::Caption, DrUITokens::InkMuted);
    SubtitleText->SetText(Subtitle);
    SubtitleText->SetVisibility(Subtitle.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
  if (TrailingText) {
    TrailingText->SetTokens(DrUITokens::Caption, DrUITokens::InkMuted);
    TrailingText->SetText(Trailing);
    TrailingText->SetVisibility(Trailing.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
  if (Chevron) {
    Chevron->SetColorToken(GetSelected() ? DrUITokens::Accent : DrUITokens::InkFaint);
    Chevron->SetVisibility(bShowChevron ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
}
