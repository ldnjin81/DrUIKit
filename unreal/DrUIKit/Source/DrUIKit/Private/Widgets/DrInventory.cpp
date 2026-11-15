#include "Widgets/DrInventory.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

// ---------------------------------------------------------------- 칸

UDrItemSlot::UDrItemSlot(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = .96f;
  Grow.Snap(1.f);
}

FName UDrItemSlot::RarityToken(EDrItemRarity Rarity) {
  switch (Rarity) {
    case EDrItemRarity::Uncommon: return DrUITokens::Success;
    case EDrItemRarity::Rare: return DrUITokens::Accent;
    case EDrItemRarity::Epic: return TEXT("ember");
    case EDrItemRarity::Legendary: return DrUITokens::Warning;
    default: return DrUITokens::Hairline;
  }
}

void UDrItemSlot::BuildDefaultTree() {
  Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
  WidgetTree->RootWidget = Frame;
  Frame->SetRenderTransformPivot(FVector2D(.5f));
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = DrUITokens::GlassStrong;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusMd;
  Surface->PaddingToken = NAME_None;
  Frame->AddChild(Surface);
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>();
  Surface->SetContent(Layers);
  auto Place = [Layers](UWidget* W, EHorizontalAlignment H, EVerticalAlignment V, FMargin Pad) {
    if (UOverlaySlot* S = Layers->AddChildToOverlay(W)) { S->SetHorizontalAlignment(H); S->SetVerticalAlignment(V); S->SetPadding(Pad); }
  };
  IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
  Place(IconImage, HAlign_Center, VAlign_Center, FMargin(10));
  IconGlyph = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("IconGlyph"));
  Place(IconGlyph, HAlign_Center, VAlign_Center, FMargin(0));
  RarityBar = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("RarityBar"));
  RarityBar->FillStrongToken = NAME_None; RarityBar->BackdropToken = NAME_None; RarityBar->StrokeToken = NAME_None;
  RarityBar->RadiusToken = DrUITokens::RadiusPill; RarityBar->PaddingToken = NAME_None;
  USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>();
  BarSize->SetHeightOverride(3.f);
  BarSize->AddChild(RarityBar);
  Place(BarSize, HAlign_Fill, VAlign_Bottom, FMargin(10, 0, 10, 5));
  CountText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("CountText"));
  Place(CountText, HAlign_Right, VAlign_Bottom, FMargin(0, 0, 6, 8));
  EquippedMark = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("EquippedMark"));
  EquippedMark->SetText(FText::FromString(TEXT("E")));
  Place(EquippedMark, HAlign_Left, VAlign_Top, FMargin(6, 4, 0, 0));
  NewDot = WidgetTree->ConstructWidget<UDrBadge>(UDrBadge::StaticClass(), TEXT("NewDot"));
  NewDot->Tone = EDrBadgeTone::Accent;
  NewDot->SetDot(true);
  Place(NewDot, HAlign_Right, VAlign_Top, FMargin(0, 6, 6, 0));
}

void UDrItemSlot::SetItem(const FDrItemData& InItem) { Item = InItem; RefreshState(); }

void UDrItemSlot::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) {
  Super::NativeOnAddedToFocusPath(InFocusEvent);
  OnFocusedNative.Broadcast();
}

void UDrItemSlot::ApplyVisualState() {
  if (!Surface) return;
  const FDrUIStateInputs In = GetStateInputs();
  const bool bRing = DrUIState::IsFocusRingVisible(In);
  // NEW는 한 번 보면(포커스·호버) 지운다.
  if (Item.bNew && (bFocusedNow || IsHovered())) { Item.bNew = false; OnSeen.Broadcast(); }
  if (Frame) { Frame->SetWidthOverride(SlotSize); Frame->SetHeightOverride(SlotSize); }
  const FName Rarity = RarityToken(Item.Rarity);
  Surface->FillToken = Item.bEmpty ? DrUITokens::Hairline : DrUITokens::GlassStrong;
  Surface->StrokeToken = bRing ? DrUITokens::Focus : Item.bEmpty ? NAME_None : Rarity;
  Surface->StrokeWidth = bRing ? 2.f : 1.f;
  Surface->ApplyTheme();
  Surface->SetBrushColor(VisualState == EDrUIState::Pressed ? FLinearColor(.9f, .9f, .9f) : FLinearColor::White);
  const bool bBrush = !Item.bEmpty && Item.Icon.GetResourceObject() != nullptr;
  if (IconImage) { IconImage->SetBrush(Item.Icon); IconImage->SetVisibility(bBrush ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed); }
  if (IconGlyph) {
    IconGlyph->GlyphSize = SlotSize * .42f;
    IconGlyph->Thickness = 2.5f;
    IconGlyph->SetShape(Item.Glyph);
    IconGlyph->SetColorToken(Item.Rarity == EDrItemRarity::Common ? DrUITokens::InkMuted : Rarity);
    IconGlyph->SetVisibility(!Item.bEmpty && !bBrush ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (RarityBar) {
    RarityBar->FillToken = Rarity;
    RarityBar->ApplyTheme();
    RarityBar->SetVisibility(!Item.bEmpty && Item.Rarity != EDrItemRarity::Common ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
  }
  if (CountText) {
    CountText->SetTokens(DrUITokens::Caption, DrUITokens::Ink);
    CountText->SetText(FText::AsNumber(Item.Count));
    CountText->SetVisibility(!Item.bEmpty && Item.Count > 1 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (EquippedMark) {
    EquippedMark->SetTokens(DrUITokens::Caption, DrUITokens::Accent);
    EquippedMark->SetVisibility(!Item.bEmpty && Item.bEquipped ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
  if (NewDot) NewDot->SetVisibility(!Item.bEmpty && Item.bNew ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  // 포커스 칸은 조금 커진다(게임 인벤토리에서 지금 칸을 멀리서도 알아보게). 호버는 덜.
  DriveSpring(Grow, bRing ? 1.06f : VisualState == EDrUIState::Hover ? 1.03f : 1.f);
  if (Frame && Grow.IsSettled()) Frame->SetRenderScale(FVector2D(Grow.Value));
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
}

void UDrItemSlot::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Grow.IsSettled()) {
    Grow.Step(DeltaTime);
    if (Frame) Frame->SetRenderScale(FVector2D(Grow.Value));
  }
}

// ---------------------------------------------------------------- 격자

int32 UDrItemGrid::WrapNeighbor(int32 Index, int32 Direction, int32 Count, int32 Columns, bool bWrap) {
  if (Count <= 0 || Columns <= 0 || Index < 0 || Index >= Count) return INDEX_NONE;
  const int32 Column = Index % Columns;
  const bool bEdge = Direction > 0 ? (Column == Columns - 1 || Index == Count - 1) : Column == 0;
  if (!bEdge) return Index + Direction;  // 줄 안: 엔진 공간 이동과 같다
  if (!bWrap) return INDEX_NONE;
  const int32 Next = Index + Direction;
  return Next >= 0 && Next < Count ? Next : INDEX_NONE;
}

void UDrItemGrid::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("Grid"));
  WidgetTree->RootWidget = Grid;
}

TSharedRef<SWidget> UDrItemGrid::RebuildWidget() {
  EnsureTree();
  if (!PendingItems.IsEmpty() && Slots.IsEmpty()) SetItems(PendingItems);
  return Super::RebuildWidget();
}

UDrItemSlot* UDrItemGrid::GetSlot(int32 Index) const { return Slots.IsValidIndex(Index) ? Slots[Index].Get() : nullptr; }

void UDrItemGrid::SetItems(const TArray<FDrItemData>& Items) {
  PendingItems = Items;
  EnsureTree();
  if (!Grid) return;
  Grid->ClearChildren();
  Slots.Reset();
  Grid->SetSlotPadding(FMargin(Gap * .5f));
  for (int32 I = 0; I < Items.Num(); ++I) {
    UDrItemSlot* ItemSlot = WidgetTree->ConstructWidget<UDrItemSlot>(UDrItemSlot::StaticClass());
    ItemSlot->Index = I;
    ItemSlot->SlotSize = SlotSize;
    ItemSlot->ActionHints = SlotHints;
    ItemSlot->SetItem(Items[I]);
    ItemSlot->OnClicked().AddWeakLambda(this, [this, I] { OnSlotClicked.Broadcast(I); });
    ItemSlot->OnFocusedNative.AddWeakLambda(this, [this, I] { OnSlotFocused.Broadcast(I); });
    ItemSlot->OnSeen.AddWeakLambda(this, [this, I] { if (PendingItems.IsValidIndex(I)) PendingItems[I].bNew = false; });
    ItemSlot->OnHintAction.AddUniqueDynamic(this, &ThisClass::HandleSlotHint);
    Grid->AddChildToUniformGrid(ItemSlot, I / Columns, I % Columns);
    Slots.Add(ItemSlot);
  }
  // 줄 끝에서 좌우로 넘기면 이웃 줄 끝·처음으로(엔진 공간 이동은 줄 끝에서 멈춘다).
  for (int32 I = 0; I < Slots.Num(); ++I) {
    for (const int32 Direction : {-1, 1}) {
      const int32 Column = I % Columns;
      const bool bEdge = Direction > 0 ? (Column == Columns - 1 || I == Slots.Num() - 1) : Column == 0;
      if (!bEdge) continue;
      const int32 Next = WrapNeighbor(I, Direction, Slots.Num(), Columns, bWrapRows);
      const EUINavigation Nav = Direction > 0 ? EUINavigation::Right : EUINavigation::Left;
      if (Next != INDEX_NONE) Slots[I]->SetNavigationRuleExplicit(Nav, Slots[Next]);
      else Slots[I]->SetNavigationRuleBase(Nav, EUINavigationRule::Stop);
    }
  }
}

void UDrItemGrid::HandleSlotHint(FName HintId) {
  // 행동은 포커스 칸에서 온다.
  if (const UDrItemSlot* Focused = Cast<UDrItemSlot>(UDrButtonBase::GetHintFocus()); Focused && Slots.Contains(Focused)) OnSlotHint.Broadcast(Focused->Index, HintId);
}

// ---------------------------------------------------------------- 능력치 비교

int32 UDrStatCompare::CompareDelta(const FDrStatRow& Row) {
  const float Scale = FMath::Pow(10.f, float(Row.Decimals));
  const float Delta = FMath::RoundToFloat(Row.New * Scale) - FMath::RoundToFloat(Row.Current * Scale);  // 보이는 자릿수에서 같으면 같음
  if (Delta == 0.f) return 0;
  return (Delta > 0.f) == Row.bHigherIsBetter ? 1 : -1;
}

void UDrStatCompare::SetRows(const TArray<FDrStatRow>& InRows) { Rows = InRows; Fill(); }

TSharedRef<SWidget> UDrStatCompare::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    List = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List"));
    WidgetTree->RootWidget = List;
    Fill();
  }
  return Super::RebuildWidget();
}

void UDrStatCompare::NativeConstruct() {
  Super::NativeConstruct();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrStatCompare::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrStatCompare::ApplyTheme() { Fill(); }

void UDrStatCompare::Fill() {
  if (!List || !WidgetTree) return;
  List->ClearChildren();
  auto Number = [](float Value, int32 Decimals) {
    FNumberFormattingOptions Options;
    Options.SetMinimumFractionalDigits(Decimals).SetMaximumFractionalDigits(Decimals);
    return FText::AsNumber(Value, &Options);
  };
  for (const FDrStatRow& Row : Rows) {
    const int32 Delta = CompareDelta(Row);
    const FName Tone = Delta > 0 ? DrUITokens::Success : Delta < 0 ? DrUITokens::Danger : DrUITokens::InkFaint;
    UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
    auto Add = [Line](UWidget* W, bool bFill, float MinPad) {
      if (UHorizontalBoxSlot* S = Line->AddChildToHorizontalBox(W)) {
        S->SetVerticalAlignment(VAlign_Center);
        S->SetPadding(FMargin(MinPad, 0, 0, 0));
        if (bFill) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
      }
    };
    UDrText* Label = WidgetTree->ConstructWidget<UDrText>();
    Label->SetTokens(DrUITokens::Label, DrUITokens::InkMuted);
    Label->SetText(Row.Label);
    Add(Label, true, 0.f);
    UDrText* Current = WidgetTree->ConstructWidget<UDrText>();
    Current->SetTokens(DrUITokens::Body, DrUITokens::InkMuted);
    Current->SetText(Number(Row.Current, Row.Decimals));
    Add(Current, false, 12.f);
    UDrGlyph* Arrow = WidgetTree->ConstructWidget<UDrGlyph>();
    Arrow->GlyphSize = 14.f;
    Arrow->SetShape(Delta == 0 ? EDrGlyphShape::Minus : (Row.New > Row.Current ? EDrGlyphShape::ArrowUp : EDrGlyphShape::ArrowDown));
    Arrow->SetColorToken(Tone);
    Add(Arrow, false, 8.f);
    UDrText* New = WidgetTree->ConstructWidget<UDrText>();
    New->SetTokens(DrUITokens::Body, Delta == 0 ? DrUITokens::InkMuted : Tone);
    New->SetText(Number(Row.New, Row.Decimals));
    Add(New, false, 6.f);
    if (UVerticalBoxSlot* S = List->AddChildToVerticalBox(Line)) S->SetPadding(FMargin(0, 3));
  }
}
