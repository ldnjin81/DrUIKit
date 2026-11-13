#include "Widgets/DrNavigation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrQuantityStepper.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"
#include "Widgets/DrTooltip.h"

namespace {
bool IsNavReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
}

// ---------------------------------------------------------------- 사이드바 항목

UDrSidebarItem::UDrSidebarItem(const FObjectInitializer& Initializer) : Super(Initializer) {
  PressedScale = 1.f;
  SetIsSelectable(true);
}

void UDrSidebarItem::SetContent(FName InId, const FText& InLabel, EDrGlyphShape InShape, int32 InCount) {
  ItemId = InId;
  Label = InLabel;
  Shape = InShape;
  Count = InCount;
  ApplyVisualState();
}

void UDrSidebarItem::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusMd;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(12.f, 9.f));
  WidgetTree->RootWidget = Surface;
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  UOverlay* IconLayer = WidgetTree->ConstructWidget<UOverlay>();
  Icon = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Icon"));
  Icon->GlyphSize = 18.f;
  IconLayer->AddChildToOverlay(Icon);
  // 배지는 테마를 칠할 때 스스로 보이고 숨으므로, 레일/펼침에 따른 표시는 감싼 상자에서 정한다.
  AlertBox = WidgetTree->ConstructWidget<USizeBox>();
  AlertDot = WidgetTree->ConstructWidget<UDrBadge>(UDrBadge::StaticClass(), TEXT("AlertDot"));
  AlertDot->bDot = true;
  AlertBox->AddChild(AlertDot);
  if (UOverlaySlot* S = IconLayer->AddChildToOverlay(AlertBox)) { S->SetHorizontalAlignment(HAlign_Right); S->SetVerticalAlignment(VAlign_Top); S->SetPadding(FMargin(0, -3, -3, 0)); }
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(IconLayer)) S->SetVerticalAlignment(VAlign_Center);
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(LabelText)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 0, 0)); }
  CountBox = WidgetTree->ConstructWidget<USizeBox>();
  CountBadge = WidgetTree->ConstructWidget<UDrBadge>(UDrBadge::StaticClass(), TEXT("CountBadge"));
  CountBadge->Tone = EDrBadgeTone::Neutral;
  CountBox->AddChild(CountBadge);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(CountBox)) S->SetVerticalAlignment(VAlign_Center);
}

void UDrSidebarItem::SetExpandAmount(float Amount) {
  ExpandAmount = FMath::Clamp(Amount, 0.f, 1.f);
  const bool bShowText = ExpandAmount > .05f;
  if (LabelText) { LabelText->SetVisibility(bShowText ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed); LabelText->SetRenderOpacity(ExpandAmount); }
  if (CountBox) { CountBox->SetRenderOpacity(ExpandAmount); CountBox->SetVisibility(bShowText && Count > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed); }
  // 레일에서는 숫자 대신 알림 점.
  if (AlertBox) AlertBox->SetVisibility(!bShowText && Count > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UDrSidebarItem::ApplyVisualState() {
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
  if (Surface) {
    // 바탕은 사이드바 렌즈가 깐다. 여기서는 포커스 링만.
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  const bool bStrong = GetSelected() || VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
  const FName Ink = bStrong ? DrUITokens::Ink : DrUITokens::InkMuted;
  if (Icon) { Icon->SetShape(Shape); Icon->SetColorToken(Ink); }
  if (LabelText) { LabelText->SetTokens(DrUITokens::Body, Ink); LabelText->SetText(Label); }
  if (CountBadge) CountBadge->SetCount(Count);
  SetExpandAmount(ExpandAmount);
}

// ---------------------------------------------------------------- 사이드바

void UDrSidebar::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
  WidgetTree->RootWidget = Frame;
  Panel = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Panel"));
  Panel->RadiusToken = DrUITokens::RadiusXl;
  Panel->PaddingToken = NAME_None;
  Panel->SetPadding(FMargin(8.f));
  Panel->SetElevation(DrUITokens::ShadowGlass);
  Panel->SetClipping(EWidgetClipping::ClipToBounds);  // 접히는 동안 이름이 넘치지 않게
  Frame->AddChild(Panel);
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  Panel->SetContent(Column);
  ToggleButton = WidgetTree->ConstructWidget<UDrIconButton>(UDrIconButton::StaticClass(), TEXT("ToggleButton"));
  ToggleButton->Shape = EDrGlyphShape::ChevronLeft;
  ToggleButton->OnClicked().AddWeakLambda(this, [this] { SetExpanded(!bExpanded); });
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(ToggleButton)) { S->SetHorizontalAlignment(HAlign_Left); S->SetPadding(FMargin(4, 0, 0, 8)); }
  Layers = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Layers"));
  Column->AddChildToVerticalBox(Layers);
  LensBox = WidgetTree->ConstructWidget<USizeBox>();
  Lens = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Lens"));
  Lens->FillToken = DrUITokens::GlassStrong;  // 유리 렌즈(디자인 언어 GlassSidebar)
  Lens->BackdropToken = NAME_None;
  Lens->RadiusToken = DrUITokens::RadiusMd;
  Lens->PaddingToken = NAME_None;
  Lens->ShadowToken = DrUITokens::ShadowFloat;
  LensBox->AddChild(Lens);
  LensBox->SetVisibility(ESlateVisibility::Hidden);
  if (UOverlaySlot* S = Layers->AddChildToOverlay(LensBox)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Top); }
  List = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("List"));
  if (UOverlaySlot* S = Layers->AddChildToOverlay(List)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }  // 오버레이 칸 기본은 왼쪽 정렬
  Group = NewObject<UCommonButtonGroupBase>(this);
  Group->SetSelectionRequired(true);
  Width.Snap(bExpanded ? PanelWidth : RailWidth);
  ApplyWidth();
}

TSharedRef<SWidget> UDrSidebar::RebuildWidget() {
  EnsureTree();
  return Super::RebuildWidget();
}

UDrSidebarItem* UDrSidebar::AddItem(FName ItemId, const FText& Label, EDrGlyphShape Shape, int32 Count) {
  EnsureTree();
  if (!List) return nullptr;
  UDrSidebarItem* Item = WidgetTree->ConstructWidget<UDrSidebarItem>(UDrSidebarItem::StaticClass());
  Item->SetContent(ItemId, Label, Shape, Count);
  if (UVerticalBoxSlot* S = List->AddChildToVerticalBox(Item)) S->SetPadding(FMargin(0, 0, 0, 2));
  Group->AddWidget(Item);
  UDrTooltip::Attach(this, Item, Label, FText::GetEmpty());  // 레일에서는 이름이 안 보이므로 툴팁(디자인 언어: 빼지 말 것)
  Item->OnClicked().AddWeakLambda(this, [this, Item] { HandleItemClicked(Item); });
  Items.Add(Item);
  if (!Active) { Active = Item; Group->SelectButtonAtIndex(0, false); }
  ApplyWidth();
  return Item;
}

void UDrSidebar::SelectItem(FName ItemId) {
  for (UDrSidebarItem* Item : Items)
    if (Item && Item->ItemId == ItemId) { Item->SetIsSelected(true, false); HandleItemClicked(Item); return; }
}

void UDrSidebar::HandleItemClicked(UDrSidebarItem* Item) {
  Active = Item;
  OnItemSelected.Broadcast(Item->ItemId);
}

void UDrSidebar::SetExpanded(bool bInExpanded) {
  bExpanded = bInExpanded;
  if (!bTicked || IsNavReduceMotion()) Width.Snap(bExpanded ? PanelWidth : RailWidth);
  else Width.Target = bExpanded ? PanelWidth : RailWidth;
  ApplyWidth();
}

void UDrSidebar::ApplyWidth() {
  if (Frame) Frame->SetWidthOverride(Width.Value);
  const float Amount = FMath::Clamp((Width.Value - RailWidth) / FMath::Max(1.f, PanelWidth - RailWidth), 0.f, 1.f);
  for (UDrSidebarItem* Item : Items) if (Item) Item->SetExpandAmount(Amount);
  if (ToggleButton) ToggleButton->SetRenderTransformAngle(180.f * (1.f - Amount));  // 펼침 ‹ / 레일 ›
}

void UDrSidebar::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  bTicked = true;
  if (!Width.IsSettled()) { Width.Step(DeltaTime); ApplyWidth(); }
  // 렌즈: 가리킨 항목, 없으면 활성 항목.
  UDrSidebarItem* Target = Active;
  for (UDrSidebarItem* Item : Items) if (Item && Item->IsHovered()) { Target = Item; break; }
  if (!Target || !Layers || !LensBox) return;
  const FGeometry& ItemGeo = Target->GetCachedGeometry();
  const FGeometry& LayerGeo = Layers->GetCachedGeometry();
  if (ItemGeo.GetLocalSize().Y <= 0.f) return;
  const float Y = LayerGeo.AbsoluteToLocal(ItemGeo.GetAbsolutePosition()).Y;
  const float H = ItemGeo.GetLocalSize().Y;
  if (!bLensPlaced || IsNavReduceMotion()) { LensY.Snap(Y); LensHeight.Snap(H); bLensPlaced = true; LensBox->SetVisibility(ESlateVisibility::HitTestInvisible); }
  else { LensY.Target = Y; LensHeight.Target = H; LensY.Step(DeltaTime); LensHeight.Step(DeltaTime); }
  LensBox->SetHeightOverride(LensHeight.Value);
  LensBox->SetRenderTranslation(FVector2D(0.f, LensY.Value));
}

// ---------------------------------------------------------------- 독

UDrDockItem::UDrDockItem(const FObjectInitializer& Initializer) : Super(Initializer) { PressedScale = .94f; }

void UDrDockItem::BuildDefaultTree() {
  TileBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TileBox"));
  TileBox->SetWidthOverride(48.f);
  WidgetTree->RootWidget = TileBox;
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  TileBox->AddChild(Column);
  TileLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("TileLayer"));
  USizeBox* TileSize = WidgetTree->ConstructWidget<USizeBox>();
  TileSize->SetWidthOverride(40.f);
  TileSize->SetHeightOverride(40.f);
  Tile = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Tile"));
  Tile->FillStrongToken = NAME_None;
  Tile->BackdropToken = NAME_None;
  Tile->StrokeToken = NAME_None;
  Tile->RadiusToken = DrUITokens::RadiusMd;
  Tile->PaddingToken = NAME_None;
  Tile->SetHorizontalAlignment(HAlign_Center);
  Tile->SetVerticalAlignment(VAlign_Center);
  TileSize->AddChild(Tile);
  Icon = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Icon"));
  Icon->GlyphSize = 20.f;
  Tile->SetContent(Icon);
  TileLayer->AddChildToOverlay(TileSize);
  CountBadge = WidgetTree->ConstructWidget<UDrBadge>(UDrBadge::StaticClass(), TEXT("CountBadge"));
  CountBadge->Tone = EDrBadgeTone::Alert;
  if (UOverlaySlot* S = TileLayer->AddChildToOverlay(CountBadge)) { S->SetHorizontalAlignment(HAlign_Right); S->SetVerticalAlignment(VAlign_Top); S->SetPadding(FMargin(0, -5, -5, 0)); }
  // 아래 가운데를 축으로 커진다(이웃은 자리 너비로 밀린다).
  TileLayer->SetRenderTransformPivot(FVector2D(.5f, 1.f));
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(TileLayer)) S->SetHorizontalAlignment(HAlign_Center);
  USizeBox* DotSize = WidgetTree->ConstructWidget<USizeBox>();
  DotSize->SetWidthOverride(4.f);
  DotSize->SetHeightOverride(4.f);
  RunningDot = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("RunningDot"));
  RunningDot->FillToken = DrUITokens::Ink;
  RunningDot->FillStrongToken = NAME_None; RunningDot->BackdropToken = NAME_None; RunningDot->StrokeToken = NAME_None;
  RunningDot->RadiusToken = DrUITokens::RadiusPill; RunningDot->PaddingToken = NAME_None;
  DotSize->AddChild(RunningDot);
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(DotSize)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 4, 0, 0)); }
}

void UDrDockItem::SetMagnify(float Magnify) {
  if (TileBox) TileBox->SetWidthOverride(48.f * Magnify);
  if (TileLayer) TileLayer->SetRenderScale(FVector2D(Magnify));
}

void UDrDockItem::ApplyVisualState() {
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .4f : 1.f);
  if (Tile) {
    Tile->FillToken = ColorToken;
    Tile->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Tile->StrokeWidth = 2.f;
    Tile->ApplyTheme();
    Tile->SetBrushColor(VisualState == EDrUIState::Pressed ? FLinearColor(.85f, .85f, .85f) : FLinearColor::White);
  }
  if (Icon) { Icon->SetShape(Shape); Icon->SetColorToken(DrUITokens::OnAccent); }
  if (RunningDot) { RunningDot->ApplyTheme(); RunningDot->SetVisibility(bRunning ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden); }
  if (CountBadge) CountBadge->SetCount(Count);
}

float UDrDock::MagnifyFor(int32 Distance) {
  return Distance == 0 ? 1.4f : Distance == 1 ? 1.15f : 1.f;
}

void UDrDock::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  Bar = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Bar"));
  Bar->RadiusToken = DrUITokens::RadiusPill;
  Bar->PaddingToken = NAME_None;
  Bar->SetPadding(FMargin(10.f, 8.f, 10.f, 4.f));
  Bar->SetElevation(DrUITokens::ShadowGlass);
  WidgetTree->RootWidget = Bar;
  Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
  Bar->SetContent(Row);
}

TSharedRef<SWidget> UDrDock::RebuildWidget() {
  EnsureTree();
  return Super::RebuildWidget();
}

UDrDockItem* UDrDock::AddItem(FName ItemId, const FText& Name, EDrGlyphShape Shape, FName ColorToken, bool bRunning, int32 Count) {
  EnsureTree();
  if (!Row) return nullptr;
  UDrDockItem* Item = WidgetTree->ConstructWidget<UDrDockItem>(UDrDockItem::StaticClass());
  Item->ItemId = ItemId;
  Item->Shape = Shape;
  Item->ColorToken = ColorToken;
  Item->bRunning = bRunning;
  Item->Count = Count;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Item)) S->SetVerticalAlignment(VAlign_Bottom);
  UDrTooltip::Attach(this, Item, Name, FText::GetEmpty());
  Item->OnClicked().AddWeakLambda(this, [this, ItemId] { OnItemClicked.Broadcast(ItemId); });
  Items.Add(Item);
  FDrUISpring Scale;
  Scale.Stiffness = 380.f;
  Scale.Damping = 28.f;
  Scale.Snap(1.f);
  Scales.Add(Scale);
  return Item;
}

void UDrDock::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  int32 Pointed = INDEX_NONE;
  for (int32 I = 0; I < Items.Num(); ++I) if (Items[I] && Items[I]->IsPointed()) { Pointed = I; break; }
  const bool bReduce = IsNavReduceMotion();
  for (int32 I = 0; I < Items.Num(); ++I) {
    const float Target = bReduce || Pointed == INDEX_NONE ? 1.f : MagnifyFor(FMath::Abs(I - Pointed));
    FDrUISpring& Scale = Scales[I];
    if (bReduce) Scale.Snap(1.f);
    else { Scale.Target = Target; Scale.Step(DeltaTime); }
    if (Items[I]) Items[I]->SetMagnify(Scale.Value);
  }
}

// ---------------------------------------------------------------- 하단 탭바

void UDrBottomTabButton::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusPill;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(12.f, 6.f));
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  WidgetTree->RootWidget = Surface;
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  Surface->SetContent(Column);
  Icon = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Icon"));
  Icon->GlyphSize = 20.f;
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Icon)) S->SetHorizontalAlignment(HAlign_Center);
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = DrUITokens::Caption;
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(LabelText)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 2, 0, 0)); }
}

void UDrBottomTabButton::ApplyVisualState() {
  Super::ApplyVisualState();
  if (Icon && LabelText) { Icon->SetShape(Shape); Icon->SetColorToken(LabelText->ColorToken); }
}

UDrBottomTabBar::UDrBottomTabBar(const FObjectInitializer& Initializer) : Super(Initializer) {
  bEqualWidth = true;
  TabButtonClass = UDrBottomTabButton::StaticClass();
}

bool UDrBottomTabBar::AddIconTab(FName TabId, const FText& TabLabel, EDrGlyphShape Shape, UWidget* Content) {
  PendingShapes.Add(TabId, Shape);
  const bool bAdded = AddTab(TabId, TabLabel, Content);
  PendingShapes.Remove(TabId);
  return bAdded;
}

void UDrBottomTabBar::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) {
  Super::HandleTabCreation_Implementation(TabNameID, TabButton);
  if (UDrBottomTabButton* Button = Cast<UDrBottomTabButton>(TabButton))
    if (const EDrGlyphShape* Shape = PendingShapes.Find(TabNameID)) Button->SetShape(*Shape);
}
