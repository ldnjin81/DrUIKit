#include "Widgets/DrItemSlot.h"
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
