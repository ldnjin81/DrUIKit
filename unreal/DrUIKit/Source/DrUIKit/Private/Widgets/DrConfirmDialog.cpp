#include "Widgets/DrConfirmDialog.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButton.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrConfirmDialog::UDrConfirmDialog(const FObjectInitializer& Initializer) : Super(Initializer) {
  bIsBackHandler = true;  // 뒤로 = 취소
  Presence.Stiffness = 380.f;
  Presence.Damping = 32.f;
  Presence.Snap(0.f);
}

TOptional<FUIInputConfig> UDrConfirmDialog::GetDesiredInputConfig() const { return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture); }

void UDrConfirmDialog::SetContent(const FText& InTitle, const FText& InBody, const FText& InConfirmLabel, const FText& InCancelLabel, bool bInDestructive) {
  Title = InTitle;
  Body = InBody;
  ConfirmLabel = InConfirmLabel;
  CancelLabel = InCancelLabel;
  bDestructive = bInDestructive;
  ApplyContent();
}

TSharedRef<SWidget> UDrConfirmDialog::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;
    Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Scrim"));  // 뒤 누름을 막는다
    if (UOverlaySlot* S = Root->AddChildToOverlay(Scrim)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
    USizeBox* PanelSize = WidgetTree->ConstructWidget<USizeBox>();
    PanelSize->SetWidthOverride(420.f);
    Panel = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Panel"));
    Panel->FillToken = DrUITokens::GlassStrong;
    Panel->BackdropToken = TEXT("backdrop-strong");  // 디자인 시스템: 모달
    Panel->RadiusToken = DrUITokens::RadiusXl;
    Panel->PaddingToken = NAME_None;
    Panel->SetPadding(FMargin(24.f, 22.f, 24.f, 20.f));
    Panel->SetElevation(DrUITokens::ShadowGlass);
    PanelSize->AddChild(Panel);
    if (UOverlaySlot* S = Root->AddChildToOverlay(PanelSize)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Center); }
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Panel->SetContent(Column);
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink);
    Column->AddChildToVerticalBox(TitleText);
    BodyText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("BodyText"));
    BodyText->SetTokens(DrUITokens::Body, DrUITokens::InkMuted);
    BodyText->SetAutoWrapText(true);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(BodyText)) S->SetPadding(FMargin(0, 8, 0, 20));
    UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
    if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(WidgetTree->ConstructWidget<USpacer>())) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    CancelButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("CancelButton"));
    CancelButton->Variant = EDrButtonVariant::Secondary;
    CancelButton->OnClicked().AddWeakLambda(this, [this] { Resolve(false); });
    Buttons->AddChildToHorizontalBox(CancelButton);
    ConfirmButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("ConfirmButton"));
    ConfirmButton->OnClicked().AddWeakLambda(this, [this] { Resolve(true); });
    if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(ConfirmButton)) S->SetPadding(FMargin(8, 0, 0, 0));
    Column->AddChildToVerticalBox(Buttons);
    ApplyContent();
  }
  return Super::RebuildWidget();
}

void UDrConfirmDialog::ApplyContent() {
  if (TitleText) TitleText->SetText(Title);
  if (BodyText) { BodyText->SetText(Body); BodyText->SetVisibility(Body.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible); }
  if (ConfirmButton) {
    ConfirmButton->Variant = bDestructive ? EDrButtonVariant::Destructive : EDrButtonVariant::Primary;
    ConfirmButton->SetLabel(ConfirmLabel.IsEmpty() ? NSLOCTEXT("DrConfirmDialog", "Confirm", "확인") : ConfirmLabel);
  }
  if (CancelButton) CancelButton->SetLabel(CancelLabel.IsEmpty() ? NSLOCTEXT("DrConfirmDialog", "Cancel", "취소") : CancelLabel);
  ApplyPresence();
}

void UDrConfirmDialog::NativeOnActivated() {
  Super::NativeOnActivated();
  bResolved = false;
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Subsystem && Subsystem->IsReduceMotion()) Presence.Snap(1.f);
  else {
    Presence.Snap(0.f);
    Presence.Target = 1.f;
    if (!Ticker.IsValid()) Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::StepPresence));
  }
  ApplyPresence();
}

void UDrConfirmDialog::NativeOnDeactivated() {
  if (Ticker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
  Ticker.Reset();
  Super::NativeOnDeactivated();
}

UWidget* UDrConfirmDialog::NativeGetDesiredFocusTarget() const {
  // 파괴적 행동은 취소에 첫 포커스(Enter·A를 눌러도 지워지지 않게).
  return bDestructive ? static_cast<UWidget*>(CancelButton) : static_cast<UWidget*>(ConfirmButton);
}

bool UDrConfirmDialog::NativeOnHandleBackAction() {
  Resolve(false);
  return true;
}

void UDrConfirmDialog::Resolve(bool bConfirmed) {
  if (bResolved) return;
  bResolved = true;
  OnResult.Broadcast(bConfirmed);
  if (OnResultNative) { TFunction<void(bool)> Callback = MoveTemp(OnResultNative); Callback(bConfirmed); }
  DeactivateWidget();  // 스택이 빼낸다
}

void UDrConfirmDialog::ApplyPresence() {
  const float Amount = FMath::Clamp(Presence.Value, 0.f, 1.f);
  if (Scrim) Scrim->SetBrushColor(UDrUIThemeSubsystem::Current().GetColor(TEXT("scrim")) * FLinearColor(1.f, 1.f, 1.f, Amount));
  if (Panel && Panel->GetParent()) {
    Panel->GetParent()->SetRenderOpacity(Amount);
    Panel->GetParent()->SetRenderScale(FVector2D(.96f + .04f * Presence.Value));  // 살짝 커지며 나타난다
  }
}

bool UDrConfirmDialog::StepPresence(float DeltaTime) {
  const bool bSettled = Presence.Step(FMath::Min(DeltaTime, .1f));
  ApplyPresence();
  if (bSettled) Ticker.Reset();
  return !bSettled;
}
