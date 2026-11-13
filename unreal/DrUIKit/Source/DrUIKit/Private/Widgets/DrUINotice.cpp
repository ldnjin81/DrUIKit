#include "Widgets/DrUINotice.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Containers/Ticker.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUIButton.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"

namespace {
bool IsReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
}

// ---------------------------------------------------------------- 공통 틀

TSharedRef<SWidget> UDrUINoticeBase::RebuildWidget() {
  // 내용(Show·SetContent)은 트리보다 먼저 들어올 수 있으므로 ApplyTheme이 보관한 값으로 다시 쓴다.
  if (WidgetTree && !WidgetTree->RootWidget) { BuildDefaultTree(); ApplyTheme(); }
  ApplyPresence();
  return Super::RebuildWidget();
}

void UDrUINoticeBase::NativeOnInitialized() {
  Super::NativeOnInitialized();
  Presence.Stiffness = 320.f;
  Presence.Damping = 30.f;
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  ApplyTheme();
}

void UDrUINoticeBase::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrUINoticeBase::Enter(float Seconds) {
  Remaining = Seconds;
  if (!bShown) Presence.Snap(0.f);
  bShown = true;
  if (IsReduceMotion()) Presence.Snap(1.f);
  else Presence.Target = 1.f;
  ApplyPresence();
}

void UDrUINoticeBase::Leave() {
  if (!bShown) return;
  Remaining = 0.f;
  Presence.Target = 0.f;
  if (IsReduceMotion()) Presence.Snap(0.f);
  ApplyPresence();
  if (Presence.IsSettled()) { bShown = false; ApplyPresence(); HandleLeft(); }
}

void UDrUINoticeBase::ApplyPresence() {
  SetRenderOpacity(FMath::Clamp(Presence.Value, 0.f, 1.f));
  FWidgetTransform Transform = GetRenderTransform();
  Transform.Translation = EnterOffset * (1.f - Presence.Value);  // 스프링이 넘치면 살짝 지나쳤다 돌아온다
  SetRenderTransform(Transform);
  SetVisibility(!bShown ? ESlateVisibility::Collapsed : IsLeaving() ? ESlateVisibility::HitTestInvisible : ShownVisibility());
}

void UDrUINoticeBase::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!bShown) return;
  if (Remaining > 0.f && !IsLeaving() && !bHoldTimer) {
    Remaining -= DeltaTime;
    if (Remaining <= 0.f) { Leave(); if (!bShown) return; }
  }
  if (!Presence.IsSettled()) {
    if (IsReduceMotion()) Presence.Snap(Presence.Target);
    else Presence.Step(DeltaTime);
    ApplyPresence();
  }
  if (IsLeaving() && Presence.IsSettled()) { bShown = false; ApplyPresence(); HandleLeft(); }
}

// ---------------------------------------------------------------- 토스트

void UDrUIToast::ToneGlyph(EDrUINoticeTone InTone, EDrUIGlyphShape& OutShape, FName& OutColor) {
  // 색만으로 구분하지 않도록 모양도 다르게(디자인 시스템: 아이콘·문구를 함께).
  switch (InTone) {
  case EDrUINoticeTone::Success: OutShape = EDrUIGlyphShape::Check; OutColor = DrUITokens::Success; break;
  case EDrUINoticeTone::Warning: OutShape = EDrUIGlyphShape::Alert; OutColor = DrUITokens::Warning; break;
  case EDrUINoticeTone::Danger: OutShape = EDrUIGlyphShape::Close; OutColor = DrUITokens::Danger; break;
  default: OutShape = EDrUIGlyphShape::Dot; OutColor = DrUITokens::Accent; break;
  }
}

void UDrUIToast::SetContent(const FText& InMessage, EDrUINoticeTone InTone) {
  Message = InMessage;
  Tone = InTone;
  ApplyTheme();
}

void UDrUIToast::BuildDefaultTree() {
  USizeBox* Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
  Frame->SetMaxDesiredWidth(420.f);
  WidgetTree->RootWidget = Frame;
  Surface = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = DrUITokens::GlassStrong;
  Surface->BackdropToken = DrUITokens::BackdropSoft;
  Surface->RadiusToken = DrUITokens::RadiusPill;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(14.f, 9.f, 18.f, 9.f));
  Frame->AddChild(Surface);
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  Icon = WidgetTree->ConstructWidget<UDrUIGlyph>(UDrUIGlyph::StaticClass(), TEXT("Icon"));
  Icon->GlyphSize = 16.f;
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Icon)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 8, 0)); }
  MessageText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("MessageText"));
  MessageText->SetAutoWrapText(true);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(MessageText)) { S->SetVerticalAlignment(VAlign_Center); S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
}

void UDrUIToast::ApplyTheme() {
  if (Surface) Surface->ApplyTheme();
  if (Icon) {
    EDrUIGlyphShape Shape;
    FName Color;
    ToneGlyph(Tone, Shape, Color);
    Icon->SetShape(Shape);
    Icon->SetColorToken(Color);
  }
  if (MessageText) {
    MessageText->SetTokens(DrUITokens::Body, DrUITokens::Ink);
    MessageText->SetText(Message);
  }
}

// ---------------------------------------------------------------- 토스트 쌓개

TSharedRef<SWidget> UDrUIToastStack::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Stack"));
    WidgetTree->RootWidget = Stack;
  }
  // 빈 자리는 눌림을 막지 않는다(토스트 자신도 눌리지 않음).
  SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  return Super::RebuildWidget();
}

int32 UDrUIToastStack::GetActiveCount() const {
  int32 Count = 0;
  if (Stack)
    for (UWidget* Child : Stack->GetAllChildren())
      if (const UDrUIToast* Toast = Cast<UDrUIToast>(Child); Toast && Toast->IsShown() && !Toast->IsLeaving()) ++Count;
  return Count;
}

UDrUIToast* UDrUIToastStack::Push(const FText& Message, EDrUINoticeTone Tone, float Seconds) {
  if (!Stack) TakeWidget();  // 아직 화면에 붙기 전이면 기본 구성을 먼저 만든다
  if (!Stack) return nullptr;
  UDrUIToast* Toast = CreateWidget<UDrUIToast>(this, ToastClass ? ToastClass.Get() : UDrUIToast::StaticClass());
  if (!Toast) return nullptr;
  Toast->SetContent(Message, Tone);
  if (UVerticalBoxSlot* S = Stack->AddChildToVerticalBox(Toast)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, Gap)); }
  // 다 사라진 토스트는 다음 틱에 치운다(토스트 자신의 틱 안에서 부모를 바꾸지 않도록).
  Toast->OnLeft.AddWeakLambda(this, [this, Weak = TWeakObjectPtr<UDrUIToast>(Toast)] {
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [Weak](float) {
      if (UDrUIToast* Done = Weak.Get(); Done && !Done->IsShown()) Done->RemoveFromParent();
      return false;
    }));
  });
  // 넘치면 가장 오래된(위쪽) 것부터 물러난다.
  int32 Over = GetActiveCount() - (MaxVisible - 1);
  for (UWidget* Child : Stack->GetAllChildren()) {
    if (Over <= 0) break;
    if (UDrUIToast* Old = Cast<UDrUIToast>(Child); Old && Old != Toast && Old->IsShown() && !Old->IsLeaving()) { Old->Leave(); --Over; }
  }
  Toast->Enter(Seconds);
  return Toast;
}

// ---------------------------------------------------------------- 스낵바

UDrUISnackbar::UDrUISnackbar(const FObjectInitializer& Initializer) : Super(Initializer) {
  EnterOffset = FVector2D(0.f, 16.f);  // 아래에서 올라온다
}

void UDrUISnackbar::NativeOnInitialized() {
  // 위젯 블루프린트가 ActionButton을 두었으면 여기서 잇는다(기본 구성은 BuildDefaultTree에서).
  if (ActionButton) ActionButton->OnClicked().AddUObject(this, &ThisClass::HandleActionClicked);
  Super::NativeOnInitialized();
}

void UDrUISnackbar::BuildDefaultTree() {
  USizeBox* Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
  Frame->SetMinDesiredWidth(320.f);
  Frame->SetMaxDesiredWidth(560.f);
  WidgetTree->RootWidget = Frame;
  Surface = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Surface"));
  // 진한 알약(chip): 유리 위 어디서나 눈에 띄고 투명도 설정을 따르지 않는다.
  Surface->FillToken = DrUITokens::Chip;
  Surface->FillStrongToken = NAME_None;
  Surface->StrokeToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusLg;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(18.f, 8.f, 8.f, 8.f));
  Frame->AddChild(Surface);
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  Surface->SetContent(Row);
  MessageText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("MessageText"));
  MessageText->SetAutoWrapText(true);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(MessageText)) { S->SetVerticalAlignment(VAlign_Center); S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetPadding(FMargin(0, 6, 12, 6)); }
  ActionButton = WidgetTree->ConstructWidget<UDrUIButton>(UDrUIButton::StaticClass(), TEXT("ActionButton"));
  ActionButton->Variant = EDrUIButtonVariant::Primary;  // accent 채움 + on-accent 글자: chip 위에서 밝은·어두운 테마 모두 읽힌다
  ActionButton->OnClicked().AddUObject(this, &ThisClass::HandleActionClicked);
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ActionButton)) S->SetVerticalAlignment(VAlign_Center);
}

void UDrUISnackbar::ApplyTheme() {
  if (Surface) Surface->ApplyTheme();
  if (MessageText) {
    MessageText->SetTokens(DrUITokens::Body, DrUITokens::OnChip);
    MessageText->SetText(Current.Message);
  }
  if (ActionButton) {
    ActionButton->SetVisibility(Current.ActionLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    ActionButton->SetLabel(Current.ActionLabel);
  }
}

int32 UDrUISnackbar::Show(const FText& Message, const FText& ActionLabel, float Seconds) {
  FRequest Request;
  Request.Id = NextId++;
  Request.Message = Message;
  Request.ActionLabel = ActionLabel;
  Request.Seconds = Seconds;
  if (IsShown()) Queue.Add(Request);  // 보이는(물러나는 중 포함) 동안은 차례를 기다린다
  else Present(Request);
  return Request.Id;
}

void UDrUISnackbar::Present(const FRequest& Request) {
  Current = Request;
  bByAction = false;
  bHoldTimer = false;
  ApplyTheme();
  Enter(Request.Seconds);
}

void UDrUISnackbar::Dismiss() { Leave(); }

void UDrUISnackbar::HandleLeft() {
  const int32 Id = Current.Id;
  const bool bWasAction = bByAction;
  Current = FRequest();
  // 다음 요청을 먼저 띄운 뒤 알린다(알림을 받은 쪽이 Show하면 줄 뒤에 서도록).
  if (Queue.Num() > 0) {
    const FRequest Next = Queue[0];
    Queue.RemoveAt(0);
    Present(Next);
  } else {
    ApplyTheme();
  }
  OnDismissed.Broadcast(Id, bWasAction);
}

void UDrUISnackbar::HandleActionClicked() {
  if (!IsShown() || IsLeaving()) return;
  bByAction = true;
  OnAction.Broadcast(Current.Id);
  Leave();
}

void UDrUISnackbar::NativeOnMouseEnter(const FGeometry& Geometry, const FPointerEvent& Event) {
  Super::NativeOnMouseEnter(Geometry, Event);
  bHoldTimer = true;  // 읽는 중에는 사라지지 않는다
}

void UDrUISnackbar::NativeOnMouseLeave(const FPointerEvent& Event) {
  Super::NativeOnMouseLeave(Event);
  bHoldTimer = false;
}
