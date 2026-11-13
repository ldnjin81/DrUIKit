#include "Widgets/DrOverlays.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MenuAnchor.h"
#include "Core/DrUIMenu.h"
#include "Components/Overlay.h"
#include "Components/ScrollBox.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CommonButtonBase.h"
#include "Core/DrUIDraw.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButton.h"
#include "Widgets/DrQuantityStepper.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"
#include "Widgets/SLeafWidget.h"

namespace {
bool IsOverlayReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
UDrSurface* MakePlainSurface(UWidgetTree* Tree, FName Name, FName Fill, FName Radius) {
  UDrSurface* S = Tree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), Name);
  S->FillToken = Fill;
  S->FillStrongToken = NAME_None;
  S->BackdropToken = NAME_None;
  S->StrokeToken = NAME_None;
  S->RadiusToken = Radius;
  S->PaddingToken = NAME_None;
  return S;
}
}

// ---------------------------------------------------------------- 배너

void UDrBanner::SetContent(const FText& InTitle, const FText& InMessage, EDrNoticeTone InTone, const FText& InActionLabel) {
  Title = InTitle;
  Message = InMessage;
  Tone = InTone;
  ActionLabel = InActionLabel;
  ApplyTheme();
}

TSharedRef<SWidget> UDrBanner::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetClipping(EWidgetClipping::ClipToBounds);
    WidgetTree->RootWidget = Frame;
    Surface = MakePlainSurface(WidgetTree, TEXT("Surface"), DrUITokens::GlassStrong, DrUITokens::RadiusLg);
    Surface->StrokeToken = DrUITokens::GlassStroke;
    Surface->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
    Frame->AddChild(Surface);
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Surface->SetContent(Row);
    USizeBox* StripeBox = WidgetTree->ConstructWidget<USizeBox>();
    StripeBox->SetWidthOverride(4.f);
    Stripe = MakePlainSurface(WidgetTree, TEXT("Stripe"), DrUITokens::Accent, DrUITokens::RadiusXs);  // 알약(높이 절반 반경)이면 4px 폭에서 타원이 된다
    StripeBox->AddChild(Stripe);
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(StripeBox)) { S->SetVerticalAlignment(VAlign_Fill); S->SetPadding(FMargin(6, 8, 12, 8)); }
    Icon = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Icon"));
    Icon->GlyphSize = 18.f;
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Icon)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 0, 10, 0)); }
    UVerticalBox* Texts = WidgetTree->ConstructWidget<UVerticalBox>();
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    Texts->AddChildToVerticalBox(TitleText);
    MessageText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("MessageText"));
    Texts->AddChildToVerticalBox(MessageText);
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Texts)) { S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(0, 10)); }
    ActionButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("ActionButton"));
    ActionButton->Variant = EDrButtonVariant::Secondary;
    ActionButton->OnClicked().AddWeakLambda(this, [this] { OnAction.Broadcast(); });
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ActionButton)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(12, 0, 4, 0)); }
    CloseButton = WidgetTree->ConstructWidget<UDrIconButton>(UDrIconButton::StaticClass(), TEXT("CloseButton"));
    CloseButton->Shape = EDrGlyphShape::Close;
    CloseButton->OnClicked().AddWeakLambda(this, [this] { Dismiss(); });
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(CloseButton)) S->SetVerticalAlignment(VAlign_Center);
    Presence.Snap(1.f);
    ApplyTheme();
  }
  return Super::RebuildWidget();
}

void UDrBanner::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrBanner::ApplyTheme() {
  EDrGlyphShape Shape;
  FName Color;
  UDrToast::ToneGlyph(Tone, Shape, Color);
  if (Surface) Surface->ApplyTheme();
  if (Stripe) { Stripe->FillToken = Color; Stripe->ApplyTheme(); }
  if (Icon) { Icon->SetShape(Shape); Icon->SetColorToken(Color); }
  if (TitleText) { TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink); TitleText->SetText(Title); TitleText->SetVisibility(Title.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible); }
  if (MessageText) { MessageText->SetTokens(DrUITokens::Body, DrUITokens::InkMuted); MessageText->SetText(Message); }
  if (ActionButton) { ActionButton->SetLabel(ActionLabel); ActionButton->SetVisibility(ActionLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible); }
  if (CloseButton) CloseButton->SetVisibility(bDismissible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UDrBanner::Show() {
  SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  if (IsOverlayReduceMotion()) Presence.Snap(1.f);
  else Presence.Target = 1.f;
  ApplyPresence();
}

void UDrBanner::Dismiss() {
  if (Frame && Presence.Value >= .999f) FullHeight = Frame->GetDesiredSize().Y;  // 접기 시작할 높이
  if (IsOverlayReduceMotion()) Presence.Snap(0.f);
  else Presence.Target = 0.f;
  ApplyPresence();
}

void UDrBanner::ApplyPresence() {
  if (!Frame) return;
  const float Amount = FMath::Clamp(Presence.Value, 0.f, 1.f);
  SetRenderOpacity(Amount);
  if (Amount >= 1.f && Presence.IsSettled()) Frame->ClearHeightOverride();
  else Frame->SetHeightOverride(FullHeight * Amount);
  if (Amount <= 0.f && Presence.IsSettled() && GetVisibility() != ESlateVisibility::Collapsed) {
    SetVisibility(ESlateVisibility::Collapsed);
    OnDismissed.Broadcast();
  }
}

void UDrBanner::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Presence.IsSettled()) { Presence.Step(DeltaTime); ApplyPresence(); }
}

// ---------------------------------------------------------------- 바텀시트

UDrBottomSheet::UDrBottomSheet(const FObjectInitializer& Initializer) : Super(Initializer) {
  SetIsFocusable(true);  // Esc·패드 B를 받는다
  Presence.Snap(0.f);
}

TSharedRef<SWidget> UDrBottomSheet::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;
    Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Scrim"));
    Scrim->SetVisibility(ESlateVisibility::HitTestInvisible);  // 누름은 이 위젯이 받아 판 밖이면 닫는다
    if (UOverlaySlot* S = Root->AddChildToOverlay(Scrim)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
    USizeBox* SheetBox = WidgetTree->ConstructWidget<USizeBox>();
    SheetBox->SetWidthOverride(SheetWidth);
    Sheet = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Sheet"));
    Sheet->FillToken = DrUITokens::GlassStrong;
    Sheet->BackdropToken = TEXT("backdrop-strong");  // 디자인 시스템: 사진 위 큰 창·모달
    Sheet->RadiusToken = DrUITokens::RadiusXl;
    Sheet->PaddingToken = NAME_None;
    Sheet->SetPadding(FMargin(20.f, 10.f, 20.f, 24.f));
    Sheet->SetElevation(DrUITokens::ShadowGlass);
    SheetBox->AddChild(Sheet);
    if (UOverlaySlot* S = Root->AddChildToOverlay(SheetBox)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Bottom); S->SetPadding(FMargin(0, 0, 0, 16)); }
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Sheet->SetContent(Column);
    USizeBox* GrabBox = WidgetTree->ConstructWidget<USizeBox>();
    GrabBox->SetWidthOverride(36.f);
    GrabBox->SetHeightOverride(4.f);
    GrabBox->AddChild(MakePlainSurface(WidgetTree, TEXT("Grabber"), DrUITokens::InkFaint, DrUITokens::RadiusPill));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(GrabBox)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 14)); }
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(TitleText)) S->SetPadding(FMargin(0, 0, 0, 10));
    Body = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Body"));
    Body->SetBrush(FSlateNoResource());
    Body->SetPadding(FMargin(0.f));
    if (PendingBody) Body->SetContent(PendingBody);
    Column->AddChildToVerticalBox(Body);
    if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
      ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyPresence(); });
    ApplyPresence();
  }
  return Super::RebuildWidget();
}

void UDrBottomSheet::SetBody(UWidget* Content) {
  PendingBody = Content;
  if (Body) Body->SetContent(Content);
}

void UDrBottomSheet::Open() {
  bOpen = true;
  DragY = 0.f;
  if (IsOverlayReduceMotion()) Presence.Snap(1.f);
  else Presence.Target = 1.f;
  ApplyPresence();
  SetFocus();
}

void UDrBottomSheet::Close() {
  if (!bOpen) return;
  bOpen = false;
  bDragging = false;
  if (IsOverlayReduceMotion()) Presence.Snap(0.f);
  else Presence.Target = 0.f;
  ApplyPresence();
  OnClosed.Broadcast();
}

void UDrBottomSheet::ApplyPresence() {
  if (!Sheet || !Scrim) return;
  const float Amount = FMath::Clamp(Presence.Value, 0.f, 1.f);
  const float Height = Sheet->GetDesiredSize().Y + 16.f;
  Sheet->SetRenderTranslation(FVector2D(0.f, (1.f - Presence.Value) * Height + DragY));  // 아래에서 올라온다 + 끄는 만큼
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Scrim->SetBrushColor(Theme.GetColor(TEXT("scrim")) * FLinearColor(1.f, 1.f, 1.f, Amount));
  if (TitleText) { TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink); TitleText->SetText(Title); }
  const bool bVisible = bOpen || !Presence.IsSettled() || Amount > 0.f;
  SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UDrBottomSheet::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Presence.IsSettled()) { Presence.Step(DeltaTime); ApplyPresence(); }
}

FReply UDrBottomSheet::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (!bOpen || Event.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Handled();
  const FGeometry& SheetGeo = Sheet->GetCachedGeometry();
  if (!SheetGeo.IsUnderLocation(Event.GetScreenSpacePosition())) { Close(); return FReply::Handled(); }  // 판 밖 = 닫기
  // 판 위쪽(손잡이·제목 줄)을 잡으면 끌어서 닫을 수 있다.
  if (SheetGeo.AbsoluteToLocal(Event.GetScreenSpacePosition()).Y < 48.f) {
    bDragging = true;
    DragStartY = Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition()).Y;
    return FReply::Handled().CaptureMouse(TakeWidget());
  }
  return FReply::Handled();
}

FReply UDrBottomSheet::NativeOnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (!bDragging) return Super::NativeOnMouseMove(Geometry, Event);
  DragY = FMath::Max(0.f, Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition()).Y - DragStartY);  // 아래로만
  ApplyPresence();
  return FReply::Handled();
}

FReply UDrBottomSheet::NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (!bDragging) return Super::NativeOnMouseButtonUp(Geometry, Event);
  bDragging = false;
  const float Height = Sheet ? Sheet->GetDesiredSize().Y : 0.f;
  if (ShouldDismissDrag(DragY, Height)) {
    // 끈 자리에서 이어 내려가도록 나타남 값에 끈 거리를 옮긴다.
    Presence.Value = FMath::Clamp(1.f - DragY / FMath::Max(Height, 1.f), 0.f, 1.f);
    DragY = 0.f;
    Close();
  } else {
    DragY = 0.f;
    ApplyPresence();
  }
  return FReply::Handled().ReleaseMouseCapture();
}

FReply UDrBottomSheet::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (bOpen && (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_FaceButton_Right)) { Close(); return FReply::Handled(); }
  return Super::NativeOnKeyDown(Geometry, Event);
}

// ---------------------------------------------------------------- 코치마크 막

class SDrCoachScrim : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrCoachScrim) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrCoachScrim::Pulse)); }
  FVector2f HolePos = FVector2f::ZeroVector, HoleSize = FVector2f::ZeroVector;
  float Radius = 12.f;
  bool bReduceMotion = false;
  FLinearColor Scrim = FLinearColor(0, 0, 0, .55f), Ring = FLinearColor::Blue;
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    const float X0 = HolePos.X, Y0 = HolePos.Y, X1 = HolePos.X + HoleSize.X, Y1 = HolePos.Y + HoleSize.Y;
    const float R = FMath::Clamp(Radius, 0.f, FMath::Min(HoleSize.X, HoleSize.Y) * .5f);
    TArray<FVector2f> P;
    TArray<SlateIndex> I;
    auto Quad = [&](FVector2f A, FVector2f B) {  // 축에 나란한 사각형(왼쪽 위, 오른쪽 아래)
      if (B.X <= A.X || B.Y <= A.Y) return;
      const SlateIndex N = P.Num();
      P.Append({A, FVector2f(B.X, A.Y), B, FVector2f(A.X, B.Y)});
      I.Append({N, SlateIndex(N + 1), SlateIndex(N + 2), N, SlateIndex(N + 2), SlateIndex(N + 3)});
    };
    // 구멍의 바깥 사각형 둘레 네 띠.
    Quad(FVector2f(0.f), FVector2f(Size.X, Y0));
    Quad(FVector2f(0.f, Y1), Size);
    Quad(FVector2f(0.f, Y0), FVector2f(X0, Y1));
    Quad(FVector2f(X1, Y0), FVector2f(Size.X, Y1));
    // 네 모서리: 사각 모서리 점에서 둥근 호까지 부채.
    const FVector2f Corners[4] = {{X0, Y0}, {X1, Y0}, {X1, Y1}, {X0, Y1}};
    const FVector2f Centers[4] = {{X0 + R, Y0 + R}, {X1 - R, Y0 + R}, {X1 - R, Y1 - R}, {X0 + R, Y1 - R}};
    const float Starts[4] = {180.f, 270.f, 0.f, 90.f};
    if (R > 0.f)
      for (int32 C = 0; C < 4; ++C) {
        const SlateIndex Apex = P.Num();
        P.Add(Corners[C]);
        for (int32 K = 0; K <= 8; ++K) {
          const float A = FMath::DegreesToRadians(Starts[C] + 90.f * K / 8.f);
          P.Add(Centers[C] + R * FVector2f(FMath::Cos(A), FMath::Sin(A)));
          if (K > 0) I.Append({Apex, SlateIndex(Apex + K), SlateIndex(Apex + K + 1)});
        }
      }
    TArray<FLinearColor> Colors;
    Colors.Init(Scrim * Tint, P.Num());
    DrUIDraw::Triangles(Out, LayerId, Geometry, P, Colors, I);
    // 구멍 둘레 강조 링(천천히 숨 쉬듯).
    const float Breath = bReduceMotion ? 1.f : .65f + .35f * FMath::Sin(float(FSlateApplication::Get().GetCurrentTime()) * 4.f);
    const TArray<FVector2f> RingPath = DrUIDraw::RoundedRectPath(HolePos - FVector2f(3.f), HoleSize + FVector2f(6.f), R + 3.f);
    FSlateDrawElement::MakeLines(Out, LayerId + 1, Geometry.ToPaintGeometry(), RingPath, ESlateDrawEffect::None, Ring.CopyWithNewOpacity(Ring.A * Breath) * Tint, true, 2.f);
    return LayerId + 1;
  }
private:
  EActiveTimerReturnType Pulse(double, float) { if (!bReduceMotion) Invalidate(EInvalidateWidgetReason::Paint); return EActiveTimerReturnType::Continue; }
};

void UDrCoachScrim::SetHole(FVector2D Position, FVector2D Size, float Radius) {
  if (!Slate) return;
  Slate->HolePos = FVector2f(Position);
  Slate->HoleSize = FVector2f(Size);
  Slate->Radius = Radius;
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrCoachScrim::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const FLinearColor Scrim = Theme.GetColor(TEXT("scrim"));
  Slate->Scrim = Scrim.CopyWithNewOpacity(FMath::Max(Scrim.A, .5f));  // 구멍이 또렷하게 최소 50%
  Slate->Ring = Theme.GetColor(DrUITokens::Focus);
  Slate->bReduceMotion = IsOverlayReduceMotion();
}

TSharedRef<SWidget> UDrCoachScrim::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrCoachScrim);
  ApplyTheme();
  return Slate.ToSharedRef();
}

void UDrCoachScrim::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrCoachScrim::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 코치마크

FVector2D UDrCoachMark::PlaceBubble(FVector2D HolePos, FVector2D HoleSize, FVector2D BubbleSize, FVector2D Screen, float Gap) {
  const bool bBelow = HolePos.Y + HoleSize.Y + Gap + BubbleSize.Y <= Screen.Y;
  const float Y = bBelow ? HolePos.Y + HoleSize.Y + Gap : FMath::Max(0.f, float(HolePos.Y - Gap - BubbleSize.Y));
  const float X = FMath::Clamp(float(HolePos.X), 8.f, FMath::Max(8.f, float(Screen.X - BubbleSize.X - 8.f)));
  return FVector2D(X, Y);
}

TSharedRef<SWidget> UDrCoachMark::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;
    Scrim = WidgetTree->ConstructWidget<UDrCoachScrim>(UDrCoachScrim::StaticClass(), TEXT("Scrim"));
    if (UOverlaySlot* S = Root->AddChildToOverlay(Scrim)) { S->SetHorizontalAlignment(HAlign_Fill); S->SetVerticalAlignment(VAlign_Fill); }
    USizeBox* BubbleBox = WidgetTree->ConstructWidget<USizeBox>();
    BubbleBox->SetMaxDesiredWidth(320.f);
    Bubble = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Bubble"));
    Bubble->FillToken = DrUITokens::GlassStrong;
    Bubble->BackdropToken = DrUITokens::BackdropSoft;
    Bubble->RadiusToken = DrUITokens::RadiusLg;
    Bubble->PaddingToken = NAME_None;
    Bubble->SetPadding(FMargin(16.f, 14.f));
    Bubble->SetElevation(DrUITokens::ShadowFloat);
    BubbleBox->AddChild(Bubble);
    if (UOverlaySlot* S = Root->AddChildToOverlay(BubbleBox)) { S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(VAlign_Top); }
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Bubble->SetContent(Column);
    StepText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("StepText"));
    StepText->SetTokens(TEXT("overline"), DrUITokens::InkMuted);
    Column->AddChildToVerticalBox(StepText);
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(TitleText)) S->SetPadding(FMargin(0, 4, 0, 4));
    BodyText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("BodyText"));
    BodyText->SetTokens(DrUITokens::Body, DrUITokens::InkMuted);
    BodyText->SetAutoWrapText(true);
    Column->AddChildToVerticalBox(BodyText);
    UHorizontalBox* Buttons = WidgetTree->ConstructWidget<UHorizontalBox>();
    SkipButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("SkipButton"));
    SkipButton->Variant = EDrButtonVariant::Ghost;
    SkipButton->Label = NSLOCTEXT("DrCoachMark", "Skip", "건너뛰기");
    SkipButton->OnClicked().AddWeakLambda(this, [this] { Finish(true); });
    Buttons->AddChildToHorizontalBox(SkipButton);
    if (UHorizontalBoxSlot* S = Buttons->AddChildToHorizontalBox(WidgetTree->ConstructWidget<USpacer>())) S->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    NextButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("NextButton"));
    NextButton->OnClicked().AddWeakLambda(this, [this] { Next(); });
    Buttons->AddChildToHorizontalBox(NextButton);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Buttons)) S->SetPadding(FMargin(0, 12, 0, 0));
    SetVisibility(ESlateVisibility::Collapsed);
  }
  SetIsFocusable(true);
  return Super::RebuildWidget();
}

void UDrCoachMark::AddStep(UWidget* Target, const FText& InTitle, const FText& InBody) {
  Steps.Add({Target, InTitle, InBody});
}

void UDrCoachMark::Start() {
  if (Steps.IsEmpty()) return;
  StepIndex = 0;
  bHolePlaced = false;
  SetVisibility(ESlateVisibility::Visible);  // 나머지 화면 누름을 막는다
  ShowStep();
  SetFocus();
}

void UDrCoachMark::Next() {
  if (StepIndex == INDEX_NONE) return;
  if (StepIndex + 1 >= Steps.Num()) { Finish(false); return; }
  ++StepIndex;
  ShowStep();
}

void UDrCoachMark::Finish(bool bSkipped) {
  StepIndex = INDEX_NONE;
  SetVisibility(ESlateVisibility::Collapsed);
  OnFinished.Broadcast(bSkipped);
}

void UDrCoachMark::ShowStep() {
  if (!Steps.IsValidIndex(StepIndex)) return;
  const FStep& Step = Steps[StepIndex];
  // 대상이 스크롤 상자 안에서 화면 밖이면 보이게 굴린다(밖에 있는 위젯은 위치가 갱신되지 않는다).
  if (UWidget* Target = Step.Target.Get())
    for (UWidget* Parent = Target->GetParent(); Parent; Parent = Parent->GetParent())
      if (UScrollBox* Scroll = Cast<UScrollBox>(Parent)) { Scroll->ScrollWidgetIntoView(Target, !IsOverlayReduceMotion(), EDescendantScrollDestination::Center); break; }
  if (StepText) StepText->SetText(FText::Format(NSLOCTEXT("DrCoachMark", "Step", "{0} / {1}"), FText::AsNumber(StepIndex + 1), FText::AsNumber(Steps.Num())));
  if (TitleText) TitleText->SetText(Step.Title);
  if (BodyText) BodyText->SetText(Step.Body);
  if (NextButton) NextButton->SetLabel(StepIndex + 1 >= Steps.Num() ? NSLOCTEXT("DrCoachMark", "Done", "시작하기") : NSLOCTEXT("DrCoachMark", "Next", "다음"));
  if (SkipButton) SkipButton->SetVisibility(StepIndex + 1 >= Steps.Num() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UDrCoachMark::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Steps.IsValidIndex(StepIndex)) return;
  UWidget* Target = Steps[StepIndex].Target.Get();
  if (!Target || !Scrim || !Bubble) return;
  const FGeometry& TargetGeo = Target->GetCachedGeometry();
  const FVector2D TopLeft = Geometry.AbsoluteToLocal(TargetGeo.GetAbsolutePosition()) - FVector2D(6.f);
  const FVector2D Size = TargetGeo.GetAbsoluteSize() / FMath::Max(Geometry.Scale, .01f) + FVector2D(12.f);
  FDrUISpring* Springs[4] = {&HoleX, &HoleY, &HoleW, &HoleH};
  const float Targets[4] = {float(TopLeft.X), float(TopLeft.Y), float(Size.X), float(Size.Y)};
  for (int32 I = 0; I < 4; ++I) {
    if (!bHolePlaced || IsOverlayReduceMotion()) Springs[I]->Snap(Targets[I]);
    else { Springs[I]->Target = Targets[I]; Springs[I]->Step(DeltaTime); }
  }
  bHolePlaced = true;
  const FVector2D HolePos(HoleX.Value, HoleY.Value), HoleSize(HoleW.Value, HoleH.Value);
  Scrim->SetHole(HolePos, HoleSize, UDrUIThemeSubsystem::Current().GetRadius(DrUITokens::RadiusLg));
  const FVector2D BubbleSize = Bubble->GetDesiredSize();
  if (UWidget* BubbleBox = Bubble->GetParent()) BubbleBox->SetRenderTranslation(PlaceBubble(HolePos, HoleSize, BubbleSize, FVector2D(Geometry.GetLocalSize())));
}

FReply UDrCoachMark::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (!IsRunning()) return Super::NativeOnKeyDown(Geometry, Event);
  const FKey Key = Event.GetKey();
  if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right) { Finish(true); return FReply::Handled(); }
  if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom) { Next(); return FReply::Handled(); }
  return Super::NativeOnKeyDown(Geometry, Event);
}

// ---------------------------------------------------------------- 팝오버

UDrPopoverPanel::UDrPopoverPanel(const FObjectInitializer& Initializer) : Super(Initializer) { SetIsFocusable(true); }

TSharedRef<SWidget> UDrPopoverPanel::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    // 팝업 창은 내용 크기에 딱 맞으므로, 판 바깥으로 그리는 그림자가 잘리지 않게 투명 여백을 둔다.
    UBorder* Margin = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ShadowMargin"));
    Margin->SetBrush(FSlateNoResource());
    Margin->SetPadding(FMargin(0.f, 0.f, 14.f, 22.f));  // 왼쪽·위는 0: 팝업이 누른 위젯 왼쪽 아래에 맞춰 뜨므로(그림자는 아래로 진다)
    WidgetTree->RootWidget = Margin;
    Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
    Surface->FillToken = DrUITokens::GlassStrong;
    Surface->BackdropToken = DrUITokens::BackdropSoft;
    Surface->StrokeToken = DrUITokens::GlassStroke;
    Surface->RadiusToken = DrUITokens::RadiusLg;
    Surface->PaddingToken = NAME_None;
    Surface->SetPadding(FMargin(14.f));
    Surface->SetElevation(DrUITokens::ShadowFloat);
    Margin->SetContent(Surface);
    if (PendingBody) Surface->SetContent(PendingBody);
  }
  return Super::RebuildWidget();
}

void UDrPopoverPanel::SetBody(UWidget* Content) {
  PendingBody = Content;
  if (Surface) Surface->SetContent(Content);
}

FReply UDrPopoverPanel::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::Gamepad_FaceButton_Right) { OnDismiss.ExecuteIfBound(); return FReply::Handled(); }
  return Super::NativeOnKeyDown(Geometry, Event);
}

TSharedRef<SWidget> UDrPopover::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Anchor = WidgetTree->ConstructWidget<UMenuAnchor>(UMenuAnchor::StaticClass(), TEXT("Anchor"));
    Anchor->SetPlacement(MenuPlacement_BelowAnchor);
    Anchor->FitInWindow(true);
    DrUIMenu::HideMenuBackground(Anchor);  // 엔진 메뉴 창의 진회색 배경을 끈다(판은 우리가 칠한다)
    Anchor->OnGetUserMenuContentEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDrPopover, HandleGetMenu));
    WidgetTree->RootWidget = Anchor;
    if (PendingTrigger) Anchor->SetContent(PendingTrigger);
  }
  return Super::RebuildWidget();
}

void UDrPopover::SetTrigger(UWidget* Trigger) {
  PendingTrigger = Trigger;
  if (Anchor) Anchor->SetContent(Trigger);
  // CommonUI 버튼이면 누르면 열고 닫는다(바깥을 눌러 닫힌 그 누름으로 다시 열리지 않게 엔진 판단을 따름).
  if (UCommonButtonBase* Button = Cast<UCommonButtonBase>(Trigger)) Button->OnClicked().AddWeakLambda(this, [this] { Toggle(); });
}

void UDrPopover::SetBody(UWidget* Content) {
  PendingBody = Content;
  if (Panel) Panel->SetBody(Content);
}

UUserWidget* UDrPopover::HandleGetMenu() {
  if (!Panel) {
    Panel = CreateWidget<UDrPopoverPanel>(this, UDrPopoverPanel::StaticClass());
    Panel->SetBody(PendingBody);
    Panel->OnDismiss.BindWeakLambda(this, [this] { Close(); });
  }
  return Panel;
}

void UDrPopover::Open() { if (Anchor && !Anchor->IsOpen()) Anchor->Open(true); }
void UDrPopover::Close() { if (Anchor) Anchor->Close(); }
void UDrPopover::Toggle() {
  if (!Anchor) return;
  if (Anchor->IsOpen()) Anchor->Close();
  else if (Anchor->ShouldOpenDueToClick()) Anchor->Open(true);
}
