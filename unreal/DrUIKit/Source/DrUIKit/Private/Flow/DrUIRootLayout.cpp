#include "Flow/DrUIRootLayout.h"
#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/DrNotice.h"

void UDrUIRootLayout::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Layers"));
  WidgetTree->RootWidget = Root;
  GameStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("GameStack"));
  MenuStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("MenuStack"));
  ModalStack = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(UCommonActivatableWidgetStack::StaticClass(), TEXT("ModalStack"));
  for (UCommonActivatableWidgetStack* Stack : {GameStack.Get(), MenuStack.Get(), ModalStack.Get()}) {
    Stack->SetTransitionDuration(0.f);  // 창마다 자기 등장 움직임을 가진다
    if (UOverlaySlot* S = Root->AddChildToOverlay(Stack)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
  }
  // 알림 층: 토스트는 위 가운데, 스낵바는 아래 가운데(모달보다도 위).
  Toasts = WidgetTree->ConstructWidget<UDrToastStack>(UDrToastStack::StaticClass(), TEXT("Toasts"));
  if (UOverlaySlot* S = Root->AddChildToOverlay(Toasts)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Top); S->SetPadding(FMargin(0, 24, 0, 0)); }
  Snackbar = WidgetTree->ConstructWidget<UDrSnackbar>(UDrSnackbar::StaticClass(), TEXT("Snackbar"));
  if (UOverlaySlot* S = Root->AddChildToOverlay(Snackbar)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Bottom); S->SetPadding(FMargin(0, 0, 0, 32)); }
}

TSharedRef<SWidget> UDrUIRootLayout::RebuildWidget() {
  EnsureTree();
  SetVisibility(ESlateVisibility::SelfHitTestInvisible);  // 빈 자리는 아래(게임·다른 UI)로 누름이 지나간다
  return Super::RebuildWidget();
}

UCommonActivatableWidgetStack* UDrUIRootLayout::GetStack(EDrUILayer Layer) const {
  switch (Layer) {
  case EDrUILayer::Menu: return MenuStack;
  case EDrUILayer::Modal: return ModalStack;
  default: return GameStack;
  }
}

UCommonActivatableWidget* UDrUIRootLayout::Push(EDrUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass) {
  if (!WidgetClass) return nullptr;
  if (!GameStack) TakeWidget();
  UCommonActivatableWidgetStack* Stack = GetStack(Layer);
  return Stack ? Stack->AddWidget(WidgetClass) : nullptr;
}

void UDrUIRootLayout::PopMenu() {
  if (MenuStack) if (UCommonActivatableWidget* Widget = MenuStack->GetActiveWidget()) Widget->DeactivateWidget();
}

void UDrUIRootLayout::ClearMenus() { if (MenuStack) MenuStack->ClearWidgets(); }

bool UDrUIRootLayout::HasModal() const { return ModalStack && ModalStack->GetNumWidgets() > 0; }
