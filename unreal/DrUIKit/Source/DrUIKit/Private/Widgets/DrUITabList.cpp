#include "Widgets/DrUITabList.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"

void UDrUITabButton::SetLabel(const FText& InLabel) { Label = InLabel; ApplyVisualState(); }

void UDrUITabButton::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = TEXT("radius-pill");
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(14.f, 7.f));
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  WidgetTree->RootWidget = Surface;
  LabelText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("label");
  Surface->SetContent(LabelText);
}

void UDrUITabButton::ApplyVisualState() {
  const bool bRing = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs());
  if (Surface) {
    // 선택 탭은 인디케이터가 칠하므로 바탕이 없다. 호버만 옅게.
    Surface->FillToken = !GetSelected() && (VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed) ? FName(TEXT("hairline")) : NAME_None;
    Surface->StrokeToken = bRing ? FName(TEXT("focus")) : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  if (LabelText) {
    LabelText->ColorToken = VisualState == EDrUIState::Disabled ? FName(TEXT("ink-faint")) : GetSelected() ? SelectedInk : FName(TEXT("ink-muted"));
    if (!GetSelected() && VisualState == EDrUIState::Hover) LabelText->ColorToken = TEXT("ink");
    LabelText->ApplyTheme();
    LabelText->SetText(Label);
  }
}

UDrUITabList::UDrUITabList(const FObjectInitializer& Initializer) : Super(Initializer) {
  TabButtonClass = UDrUITabButton::StaticClass();
}

void UDrUITabList::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  Track = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Track"));
  Track->RadiusToken = TEXT("radius-pill");
  Track->PaddingToken = NAME_None;
  WidgetTree->RootWidget = Track;
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>();
  Track->SetContent(Layers);
  IndicatorBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IndicatorBox"));
  Indicator = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Indicator"));
  Indicator->PaddingToken = NAME_None;
  Indicator->BackdropToken = NAME_None;
  Indicator->StrokeToken = NAME_None;
  IndicatorBox->AddChild(Indicator);
  IndicatorBox->SetVisibility(ESlateVisibility::Hidden);  // 첫 배치 전에는 숨긴다.
  Layers->AddChildToOverlay(IndicatorBox);
  Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
  if (UOverlaySlot* S = Layers->AddChildToOverlay(Row)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  ApplyTheme();
}

void UDrUITabList::ApplyTheme() {
  const bool bPill = Style == EDrUITabStyle::Pill;
  if (Track) {
    Track->FillToken = bPill ? FName(TEXT("glass")) : NAME_None;
    Track->FillStrongToken = bPill ? FName(TEXT("glass-strong")) : NAME_None;
    Track->StrokeToken = bPill ? FName(TEXT("glass-stroke")) : NAME_None;
    Track->BackdropToken = bPill ? FName(TEXT("backdrop-glass")) : NAME_None;
    Track->SetPadding(bPill ? FMargin(4.f) : FMargin(0.f));
    Track->ApplyTheme();
  }
  if (Indicator) {
    Indicator->FillToken = bPill ? TEXT("chip") : TEXT("accent");
    Indicator->FillStrongToken = NAME_None;
    Indicator->RadiusToken = TEXT("radius-pill");
    Indicator->ApplyTheme();
  }
  if (IndicatorBox) {
    if (UOverlaySlot* S = Cast<UOverlaySlot>(IndicatorBox->Slot)) { S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(bPill ? VAlign_Fill : VAlign_Bottom); }
    if (bPill) IndicatorBox->ClearHeightOverride(); else IndicatorBox->SetHeightOverride(2.f);
  }
}

TSharedRef<SWidget> UDrUITabList::RebuildWidget() {
  EnsureTree();
  return Super::RebuildWidget();
}

void UDrUITabList::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrUITabList::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

bool UDrUITabList::AddTab(FName TabId, const FText& TabLabel, UWidget* Content) {
  EnsureTree();
  PendingLabels.Add(TabId, TabLabel);
  const bool bAdded = RegisterTab(TabId, TabButtonClass ? TSubclassOf<UCommonButtonBase>(TabButtonClass) : TSubclassOf<UCommonButtonBase>(UDrUITabButton::StaticClass()), Content);
  PendingLabels.Remove(TabId);
  return bAdded;
}

void UDrUITabList::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) {
  EnsureTree();
  if (UDrUITabButton* Button = Cast<UDrUITabButton>(TabButton)) {
    Button->SelectedInk = Style == EDrUITabStyle::Pill ? TEXT("on-chip") : TEXT("ink");
    if (const FText* Found = PendingLabels.Find(TabNameID)) Button->SetLabel(*Found);
  }
  if (Row) if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(TabButton)) {
    S->SetVerticalAlignment(VAlign_Fill);
    if (bEqualWidth) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
  }
}

void UDrUITabList::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) {
  if (Row && TabButton) Row->RemoveChild(TabButton);
}

void UDrUITabList::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  UpdateIndicator(DeltaTime);
}

void UDrUITabList::UpdateIndicator(float DeltaTime) {
  if (!Row || !IndicatorBox) return;
  const UCommonButtonBase* Target = GetTabButtonBaseByID(GetSelectedTabId());
  if (!Target) { IndicatorBox->SetVisibility(ESlateVisibility::Hidden); bIndicatorPlaced = false; return; }
  const FGeometry& RowGeometry = Row->GetCachedGeometry();
  const FGeometry& TabGeometry = Target->GetCachedGeometry();
  const FVector2D TabSize = TabGeometry.GetLocalSize();
  if (TabSize.X <= 0.f) return;  // 아직 배치 전
  // 가운데 점으로 잰다: 눌림 크기 변화(가운데 기준)가 있어도 위치가 흔들리지 않는다.
  const FVector2D Center = RowGeometry.AbsoluteToLocal(TabGeometry.LocalToAbsolute(TabSize * .5f));
  const float Left = Center.X - TabSize.X * .5f;
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!bIndicatorPlaced || (Subsystem && Subsystem->IsReduceMotion())) {
    IndicatorX.Snap(Left);
    IndicatorWidth.Snap(TabSize.X);
    bIndicatorPlaced = true;
  } else {
    IndicatorX.Target = Left;
    IndicatorWidth.Target = TabSize.X;
    IndicatorX.Step(DeltaTime);
    IndicatorWidth.Step(DeltaTime);
  }
  IndicatorBox->SetWidthOverride(FMath::Max(0.f, IndicatorWidth.Value));
  IndicatorBox->SetRenderTranslation(FVector2D(IndicatorX.Value, 0.f));
  IndicatorBox->SetVisibility(ESlateVisibility::HitTestInvisible);
}

UDrUISegmented::UDrUISegmented(const FObjectInitializer& Initializer) : Super(Initializer) { bEqualWidth = true; }

void UDrUISegmented::NativeOnInitialized() {
  Super::NativeOnInitialized();
  OnTabSelected.AddUniqueDynamic(this, &ThisClass::HandleTabSelected);
}

void UDrUISegmented::ApplyTheme() {
  Super::ApplyTheme();
  // 세그먼트는 카드·패널 위에 놓이므로 흐림 없이 옅은 홈으로 둔다.
  if (Track) {
    Track->FillToken = TEXT("hairline");
    Track->FillStrongToken = NAME_None;
    Track->StrokeToken = NAME_None;
    Track->BackdropToken = NAME_None;
    Track->SetPadding(FMargin(3.f));
    Track->ApplyTheme();
  }
}

void UDrUISegmented::SetOptions(const TArray<FText>& Options) {
  RemoveAllTabs();
  for (int32 I = 0; I < Options.Num(); ++I) AddTab(OptionId(I), Options[I]);
}

void UDrUISegmented::SetSelectedIndex(int32 Index) {
  if (Index >= 0 && Index < GetTabCount()) SelectTabByID(OptionId(Index), true);
}

int32 UDrUISegmented::GetSelectedIndex() const {
  const FName Selected = GetSelectedTabId();
  for (int32 I = 0; I < GetTabCount(); ++I) if (GetTabIdAtIndex(I) == Selected) return I;
  return INDEX_NONE;
}

void UDrUISegmented::HandleTabSelected(FName TabId) {
  for (int32 I = 0; I < GetTabCount(); ++I) if (GetTabIdAtIndex(I) == TabId) { OnSelectionChanged.Broadcast(I); return; }
}
