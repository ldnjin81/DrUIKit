#include "DrGlassDemoMenu.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Flow/DrUIFlowSubsystem.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButton.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

#define LOCTEXT_NAMESPACE "DrGlassDemoMenu"

TSharedRef<SWidget> UDrGlassDemoMenu::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
    WidgetTree->RootWidget = Root;
    UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>();
    Scrim->SetBrushColor(UDrUIThemeSubsystem::Current().GetColor(TEXT("scrim")));
    if (UOverlaySlot* S = Root->AddChildToOverlay(Scrim)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
    USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>();
    Size->SetWidthOverride(480.f);
    UDrSurface* Panel = WidgetTree->ConstructWidget<UDrSurface>();
    Panel->RadiusToken = TEXT("radius-xl");
    Panel->PaddingToken = TEXT("space-6");
    Panel->SetElevation(TEXT("shadow-glass"));
    Size->AddChild(Panel);
    if (UOverlaySlot* S = Root->AddChildToOverlay(Size)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Center); }
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Column);
    UDrText* Title = WidgetTree->ConstructWidget<UDrText>();
    Title->SetTokens(TEXT("title"), TEXT("ink"));
    Title->SetText(LOCTEXT("Title", "메뉴 층 견본"));
    Column->AddChildToVerticalBox(Title);
    UDrText* Body = WidgetTree->ConstructWidget<UDrText>();
    Body->SetTokens(TEXT("body"), TEXT("ink-muted"));
    Body->SetAutoWrapText(true);
    Body->SetText(LOCTEXT("Body", "UI 흐름(UDrUIFlowSubsystem)이 메뉴 층에 띄운 UDrUIMenuWidget 파생 메뉴입니다. Esc·패드 메뉴 버튼·아래 버튼으로 뒤로 갑니다."));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Body)) S->SetPadding(FMargin(0, 8, 0, 20));
    UDrButton* Close = WidgetTree->ConstructWidget<UDrButton>();
    Close->Label = LOCTEXT("Close", "닫기(뒤로)");
    Close->OnClicked().AddWeakLambda(this, [this] { if (UDrUIFlowSubsystem* Flow = UDrUIFlowSubsystem::Get(this)) Flow->Back(); });
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Close)) S->SetHorizontalAlignment(HAlign_Right);
    FirstFocus = Close;
  }
  return Super::RebuildWidget();
}

#undef LOCTEXT_NAMESPACE
