#include "Widgets/DrItemGrid.h"
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
  // 칸 이동은 격자 안에서만: 좌우는 줄 끝에서 이웃 줄로(넘김을 끄면 멈춤), 위아래는 같은 칸 줄로(첫·마지막 줄에서 멈춤).
  // 엔진 공간 이동은 줄 끝에서 멈추거나 격자 밖으로 나가므로 칸이 직접 정한다.
  for (int32 I = 0; I < Slots.Num(); ++I) {
    TWeakObjectPtr<UDrItemGrid> WeakThis(this);
    for (const int32 Direction : {-1, 1}) {
      Slots[I]->NavTargets.Set(Direction > 0 ? EUINavigation::Right : EUINavigation::Left, [WeakThis, I, Direction]() -> UWidget* {
        UDrItemGrid* Grid = WeakThis.Get();
        if (!Grid) return nullptr;
        const int32 Next = WrapNeighbor(I, Direction, Grid->Slots.Num(), Grid->Columns, Grid->bWrapRows);
        return Next != INDEX_NONE ? Grid->GetSlot(Next) : Grid->GetSlot(I);
      });
      Slots[I]->NavTargets.Set(Direction > 0 ? EUINavigation::Down : EUINavigation::Up, [WeakThis, I, Direction]() -> UWidget* {
        UDrItemGrid* Grid = WeakThis.Get();
        if (!Grid) return nullptr;
        const int32 Next = I + Direction * Grid->Columns;
        // 격자 밖(첫 줄 위·마지막 줄 아래)은 엔진 기본(격자 바깥 부품으로 나가도 되는 화면이 많다).
        return Next >= 0 && Next < Grid->Slots.Num() ? Grid->GetSlot(Next) : nullptr;
      });
    }
  }
}

void UDrItemGrid::HandleSlotHint(FName HintId) {
  // 행동은 포커스 칸에서 온다.
  if (const UDrItemSlot* Focused = Cast<UDrItemSlot>(UDrButtonBase::GetHintFocus()); Focused && Slots.Contains(Focused)) OnSlotHint.Broadcast(Focused->Index, HintId);
}
