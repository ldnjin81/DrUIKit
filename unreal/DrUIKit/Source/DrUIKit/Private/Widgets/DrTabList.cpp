#include "Widgets/DrTabList.h"
#include "Core/DrUIFeedback.h"
#include "Core/DrUIInput.h"
#include "Widgets/DrButtonPrompt.h"
#include "Blueprint/WidgetTree.h"
#include "CommonAnimatedSwitcher.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

void UDrTabButton::SetLabel(const FText& InLabel) { Label = InLabel; ApplyVisualState(); }

void UDrTabButton::BuildDefaultTree() {
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = TEXT("radius-pill");
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(14.f, 7.f));
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  WidgetTree->RootWidget = Surface;
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  LabelText->TextStyle = TEXT("label");
  Surface->SetContent(LabelText);
}

void UDrTabButton::ApplyVisualState() {
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

UDrTabList::UDrTabList(const FObjectInitializer& Initializer) : Super(Initializer) {
  TabButtonClass = UDrTabButton::StaticClass();
}

void UDrTabList::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  // [이전 키] 트랙 [다음 키]. 키 모양은 넘기기를 켠 탭 목록에서만 보인다.
  UHorizontalBox* Frame = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Frame"));
  WidgetTree->RootWidget = Frame;
  PrevKey = WidgetTree->ConstructWidget<UDrButtonPrompt>(UDrButtonPrompt::StaticClass(), TEXT("PrevKey"));
  if (UHorizontalBoxSlot* S = Frame->AddChildToHorizontalBox(PrevKey)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 8, 0)); }
  Track = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Track"));
  Track->RadiusToken = TEXT("radius-pill");
  Track->PaddingToken = NAME_None;
  if (UHorizontalBoxSlot* S = Frame->AddChildToHorizontalBox(Track)) S->SetVerticalAlignment(VAlign_Center);
  NextKey = WidgetTree->ConstructWidget<UDrButtonPrompt>(UDrButtonPrompt::StaticClass(), TEXT("NextKey"));
  if (UHorizontalBoxSlot* S = Frame->AddChildToHorizontalBox(NextKey)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(8, 0, 0, 0)); }
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>();
  Track->SetContent(Layers);
  IndicatorBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IndicatorBox"));
  Indicator = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Indicator"));
  Indicator->PaddingToken = NAME_None;
  Indicator->BackdropToken = NAME_None;
  Indicator->StrokeToken = NAME_None;
  IndicatorBox->AddChild(Indicator);
  IndicatorBox->SetVisibility(ESlateVisibility::Hidden);  // 첫 배치 전에는 숨긴다.
  Layers->AddChildToOverlay(IndicatorBox);
  // 같은 너비: 균등 격자는 칸을 가장 긴 탭의 너비로 맞춘다(가로 상자 Fill은 전체를 나눠 긴 글자가 잘렸다).
  Row = bEqualWidth ? static_cast<UPanelWidget*>(WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("Row")))
                    : WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
  if (UOverlaySlot* S = Layers->AddChildToOverlay(Row)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  ApplyTheme();
}

void UDrTabList::ApplyTheme() {
  const bool bPill = Style == EDrTabStyle::Pill;
  const bool bKeys = bShowNavigationKeys && (bShoulderNavigation || bSubTabNavigation);
  for (UDrButtonPrompt* Key : {PrevKey.Get(), NextKey.Get()}) {
    if (!Key) continue;
    const bool bPrev = Key == PrevKey;
    if (bShoulderNavigation) Key->SetKeys(bPrev ? EKeys::Q : EKeys::E, bPrev ? EKeys::Gamepad_LeftShoulder : EKeys::Gamepad_RightShoulder, FText::GetEmpty());
    else Key->SetKeys(bPrev ? EKeys::Z : EKeys::C, bPrev ? EKeys::Gamepad_LeftTrigger : EKeys::Gamepad_RightTrigger, FText::GetEmpty());
    Key->KeyHeight = 20.f;
    Key->SetVisibility(bKeys ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  }
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

TSharedRef<SWidget> UDrTabList::RebuildWidget() {
  EnsureTree();
  return Super::RebuildWidget();
}

void UDrTabList::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrTabList::NativeConstruct() {
  Super::NativeConstruct();
  DrUIInput::RegisterTabList(this);
  ApplyTheme();
}

void UDrTabList::SetTabNavigation(bool bShoulder, bool bSubTab) {
  bShoulderNavigation = bShoulder;
  bSubTabNavigation = bSubTab;
  ApplyTheme();
}

bool UDrTabList::SelectAdjacentTab(int32 Direction, bool bFocusTab) {
  const int32 Count = GetTabCount();
  if (Count < 2 || Direction == 0) return false;
  int32 Current = 0;
  for (int32 I = 0; I < Count; ++I) if (GetTabIdAtIndex(I) == GetSelectedTabId()) { Current = I; break; }
  int32 Next = Current + (Direction > 0 ? 1 : -1);
  if (bWrapNavigation) Next = (Next + Count) % Count;
  else if (Next < 0 || Next >= Count) { DrUIFeedback::Play(this, EDrUIFeedback::Error); return false; }
  const FName Id = GetTabIdAtIndex(Next);
  SelectTabByID(Id);
  DrUIFeedback::Play(this, EDrUIFeedback::Tick);
  // 패드로 넘기면 내용이 바뀌며 포커스를 잃지 않도록 고른 탭 버튼으로 옮긴다(아래로 내려 내용에 들어간다).
  if (bFocusTab) if (UCommonButtonBase* Button = GetTabButtonBaseByID(Id)) Button->SetFocus();
  return true;
}

bool UDrTabList::OwnsTabButton(const UWidget* Button) const {
  if (!Button) return false;
  for (int32 I = 0; I < GetTabCount(); ++I) if (GetTabButtonBaseByID(GetTabIdAtIndex(I)) == Button) return true;
  return false;
}

bool UDrTabList::FocusContent() {
  UCommonAnimatedSwitcher* Switcher = GetLinkedSwitcher();
  UWidget* Target = Switcher ? DrUIInput::FindFirstFocusable(Switcher->GetActiveWidget()) : nullptr;
  if (!Target) return false;
  Target->SetFocus();
  return true;
}

void UDrTabList::NativeDestruct() {
  DrUIInput::UnregisterTabList(this);
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

bool UDrTabList::AddTab(FName TabId, const FText& TabLabel, UWidget* Content) {
  EnsureTree();
  PendingLabels.Add(TabId, TabLabel);
  const bool bAdded = RegisterTab(TabId, TabButtonClass ? TSubclassOf<UCommonButtonBase>(TabButtonClass) : TSubclassOf<UCommonButtonBase>(UDrTabButton::StaticClass()), Content);
  PendingLabels.Remove(TabId);
  return bAdded;
}

void UDrTabList::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) {
  EnsureTree();
  if (UDrTabButton* Button = Cast<UDrTabButton>(TabButton)) {
    Button->SelectedInk = Style == EDrTabStyle::Pill ? TEXT("on-chip") : TEXT("ink");
    if (const FText* Found = PendingLabels.Find(TabNameID)) Button->SetLabel(*Found);
  }
  if (UUniformGridPanel* Grid = Cast<UUniformGridPanel>(Row)) {
    if (UUniformGridSlot* S = Grid->AddChildToUniformGrid(TabButton, 0, Grid->GetChildrenCount())) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  } else if (UHorizontalBox* Box = Cast<UHorizontalBox>(Row)) {
    if (UHorizontalBoxSlot* S = Box->AddChildToHorizontalBox(TabButton)) S->SetVerticalAlignment(VAlign_Fill);
  }

}

void UDrTabList::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) {
  if (UDrButtonBase* Button = Cast<UDrButtonBase>(TabButton)) Button->ResetInteractionState();  // 다시 쓸 때 옛 포커스 링이 남지 않게
  if (!Row || !TabButton) return;
  Row->RemoveChild(TabButton);
  // 균등 격자는 칸 번호가 비지 않게 다시 매긴다.
  for (int32 I = 0; I < Row->GetChildrenCount(); ++I)
    if (UUniformGridSlot* S = Cast<UUniformGridSlot>(Row->GetChildAt(I)->Slot)) S->SetColumn(I);
}

void UDrTabList::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  UpdateIndicator(DeltaTime);
}

void UDrTabList::UpdateIndicator(float DeltaTime) {
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

UDrSegmentedControl::UDrSegmentedControl(const FObjectInitializer& Initializer) : Super(Initializer) { bEqualWidth = true; }

void UDrSegmentedControl::NativeOnInitialized() {
  Super::NativeOnInitialized();
  OnTabSelected.AddUniqueDynamic(this, &ThisClass::HandleTabSelected);
}

void UDrSegmentedControl::ApplyTheme() {
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

void UDrSegmentedControl::SetOptions(const TArray<FText>& Options) {
  RemoveAllTabs();
  for (int32 I = 0; I < Options.Num(); ++I) AddTab(OptionId(I), Options[I]);
}

void UDrSegmentedControl::SetSelectedIndex(int32 Index) {
  if (Index >= 0 && Index < GetTabCount()) SelectTabByID(OptionId(Index), true);
}

int32 UDrSegmentedControl::GetSelectedIndex() const {
  const FName Selected = GetSelectedTabId();
  for (int32 I = 0; I < GetTabCount(); ++I) if (GetTabIdAtIndex(I) == Selected) return I;
  return INDEX_NONE;
}

void UDrSegmentedControl::HandleTabSelected(FName TabId) {
  for (int32 I = 0; I < GetTabCount(); ++I) if (GetTabIdAtIndex(I) == TabId) { OnSelectionChanged.Broadcast(I); return; }
}
