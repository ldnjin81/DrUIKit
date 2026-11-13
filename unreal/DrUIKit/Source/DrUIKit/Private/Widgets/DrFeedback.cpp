#include "Widgets/DrFeedback.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/DrUIDraw.h"
#include "Core/DrUIMotion.h"
#include "Fonts/FontMeasure.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButton.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"
#include "Widgets/SLeafWidget.h"

namespace {
bool IsFeedbackReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
FSlateRoundedBoxBrush CircleBrush() {
  FSlateRoundedBoxBrush Brush(FLinearColor::White, 999.f);
  Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  return Brush;
}
/** 원 둘레(닫힌 꺾은선). */
TArray<FVector2f> CirclePath(FVector2f Center, float Radius) { return DrProgressRing::ArcPoints(Center, Radius, 0.f, 1.f, 48); }
}

// ---------------------------------------------------------------- 단계 표시

float UDrStepIndicator::StepCenterX(int32 Index, int32 Count, float Width) {
  if (Count <= 0) return 0.f;
  const float Cell = Width / Count;
  return Cell * (Index + .5f);
}

class SDrStepIndicator : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrStepIndicator) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { Fill.Snap(0.f); }
  TArray<FText> Steps;
  int32 Current = 0;
  FLinearColor Accent = FLinearColor::Blue, OnAccent = FLinearColor::White, Faint = FLinearColor::Gray, Ink = FLinearColor::Black, Muted = FLinearColor::Gray, Surface = FLinearColor::White;
  FSlateFontInfo NumberFont, LabelFont;
  FDrUISpring Fill;  // 이음선 채움(단계 번호 단위)

  void SetCurrent(int32 Index, bool bAnimate) {
    Current = Index;
    if (bAnimate) { Fill.Target = float(Index); if (!Timer.IsValid()) Timer = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrStepIndicator::Animate)); }
    else Fill.Snap(float(Index));
    Invalidate(EInvalidateWidgetReason::Paint);
  }
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(FMath::Max(1, Steps.Num()) * 110.f, 58.f); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const int32 Count = Steps.Num();
    if (Count <= 0) return LayerId;
    const FVector2f Size = Geometry.GetLocalSize();
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    const float R = 13.f, Y = R + 1.f;
    const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    // 이음선: 옅은 바탕 + 채움(지금 단계까지).
    for (int32 I = 0; I + 1 < Count; ++I) {
      const float X0 = UDrStepIndicator::StepCenterX(I, Count, Size.X) + R + 4.f, X1 = UDrStepIndicator::StepCenterX(I + 1, Count, Size.X) - R - 4.f;
      const TArray<FVector2f> Base = {FVector2f(X0, Y), FVector2f(X1, Y)};
      FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), Base, ESlateDrawEffect::None, Faint * Tint, true, 2.f);
      const float T = FMath::Clamp(Fill.Value - I, 0.f, 1.f);
      if (T > 0.f) {
        const TArray<FVector2f> Done = {FVector2f(X0, Y), FVector2f(FMath::Lerp(X0, X1, T), Y)};
        FSlateDrawElement::MakeLines(Out, LayerId + 1, Geometry.ToPaintGeometry(), Done, ESlateDrawEffect::None, Accent * Tint, true, 2.f);
      }
    }
    static const FSlateRoundedBoxBrush Circle = CircleBrush();
    for (int32 I = 0; I < Count; ++I) {
      const FVector2f Center(UDrStepIndicator::StepCenterX(I, Count, Size.X), Y);
      const bool bDone = I < Current, bNow = I == Current;
      // 원: 끝남 = 채움, 지금 = 바탕색 + 강조 링, 남음 = 바탕색 + 옅은 링.
      FSlateDrawElement::MakeBox(Out, LayerId + 2, Geometry.ToPaintGeometry(FVector2f(2.f * R), FSlateLayoutTransform(Center - FVector2f(R))), &Circle, ESlateDrawEffect::None, (bDone ? Accent : Surface) * Tint);
      if (!bDone) FSlateDrawElement::MakeLines(Out, LayerId + 3, Geometry.ToPaintGeometry(), CirclePath(Center, R - 1.f), ESlateDrawEffect::None, (bNow ? Accent : Faint) * Tint, true, 2.f);
      if (bDone) {
        for (const TArray<FVector2f>& Stroke : DrGlyph::Strokes(EDrGlyphShape::Check)) {
          TArray<FVector2f> P;
          for (const FVector2f& Q : Stroke) P.Add(Center - FVector2f(R * .8f) + Q * (R * 1.6f));
          FSlateDrawElement::MakeLines(Out, LayerId + 3, Geometry.ToPaintGeometry(), P, ESlateDrawEffect::None, OnAccent * Tint, true, 2.f);
        }
      } else {
        const FText Number = FText::AsNumber(I + 1);
        const FVector2D NumberSize = Measure->Measure(Number, NumberFont);
        FSlateDrawElement::MakeText(Out, LayerId + 3, Geometry.ToPaintGeometry(FVector2f(NumberSize), FSlateLayoutTransform(Center - FVector2f(NumberSize) * .5f)), Number, NumberFont, ESlateDrawEffect::None, (bNow ? Accent : Muted) * Tint);
      }
      const FVector2D LabelSize = Measure->Measure(Steps[I], LabelFont);
      FSlateDrawElement::MakeText(Out, LayerId + 3, Geometry.ToPaintGeometry(FVector2f(LabelSize), FSlateLayoutTransform(FVector2f(Center.X - float(LabelSize.X) * .5f, Y + R + 6.f))), Steps[I], LabelFont, ESlateDrawEffect::None, (bNow ? Ink : Muted) * Tint);
    }
    return LayerId + 3;
  }
private:
  EActiveTimerReturnType Animate(double, float DeltaTime) {
    const bool bDone = Fill.Step(DeltaTime);
    Invalidate(EInvalidateWidgetReason::Paint);
    if (bDone) { Timer.Reset(); return EActiveTimerReturnType::Stop; }
    return EActiveTimerReturnType::Continue;
  }
  TSharedPtr<FActiveTimerHandle> Timer;
};

void UDrStepIndicator::SetSteps(const TArray<FText>& InSteps) {
  Steps = InSteps;
  Current = FMath::Clamp(Current, 0, FMath::Max(0, Steps.Num() - 1));
  ApplyTheme();
}

void UDrStepIndicator::SetCurrent(int32 Index) {
  Current = FMath::Clamp(Index, 0, FMath::Max(0, Steps.Num()));  // Steps.Num()이면 전부 끝남
  if (Slate) Slate->SetCurrent(Current, !IsFeedbackReduceMotion());
}

void UDrStepIndicator::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Slate->Steps = Steps;
  Slate->Accent = Theme.GetColor(DrUITokens::Accent);
  Slate->OnAccent = Theme.GetColor(DrUITokens::OnAccent);
  Slate->Faint = Theme.GetColor(DrUITokens::InkFaint);
  Slate->Ink = Theme.GetColor(DrUITokens::Ink);
  Slate->Muted = Theme.GetColor(DrUITokens::InkMuted);
  Slate->Surface = Theme.GetColor(DrUITokens::GlassStrong).CopyWithNewOpacity(1.f);
  Slate->NumberFont = Theme.MakeFont(DrUITokens::Label);
  Slate->LabelFont = Theme.MakeFont(DrUITokens::Caption);
  Slate->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UDrStepIndicator::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrStepIndicator);
  ApplyTheme();
  Slate->SetCurrent(Current, false);
  return Slate.ToSharedRef();
}

void UDrStepIndicator::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrStepIndicator::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 완료 표시

FVector3f UDrCompletion::Timeline(float Seconds) {
  // 0~0.3초: 원이 0 → 1.1 → 1(살짝 튐), 0.2~0.55초: 체크 그리기, 0.3~0.9초: 고리가 퍼지며 사라짐.
  const float T = FMath::Max(0.f, Seconds);
  const float Grow = DrUIMotion::EaseOut(FMath::Clamp(T / .22f, 0.f, 1.f)) * 1.1f;
  const float Settle = FMath::Clamp((T - .22f) / .12f, 0.f, 1.f);
  const float CircleScale = T < .22f ? Grow : FMath::Lerp(1.1f, 1.f, Settle);
  const float Check = DrUIMotion::EaseOut(FMath::Clamp((T - .2f) / .35f, 0.f, 1.f));
  const float Ring = FMath::Clamp((T - .3f) / .6f, 0.f, 1.f);
  return FVector3f(CircleScale, Check, Ring);
}

class SDrCompletion : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrCompletion) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) {}
  float Size = 64.f;
  FLinearColor Color = FLinearColor::Green, Ink = FLinearColor::White;
  double StartTime = -1.0;  // 음수 = 비어 있음, 0 = 끝 모습
  bool bComplete = false;
  void Play(bool bInstant) {
    bComplete = true;
    StartTime = bInstant ? 0.0 : FSlateApplication::Get().GetCurrentTime();
    if (!bInstant && !Timer.IsValid()) Timer = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrCompletion::Animate));
    Invalidate(EInvalidateWidgetReason::Paint);
  }
  void Reset() { bComplete = false; Invalidate(EInvalidateWidgetReason::Paint); }
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size * 1.6f); }  // 퍼지는 고리 자리
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    if (!bComplete) return LayerId;
    const FVector2f Local = Geometry.GetLocalSize();
    const FVector2f Center = Local * .5f;
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    const FVector3f Now = StartTime <= 0.0 ? FVector3f(1.f, 1.f, 1.f) : UDrCompletion::Timeline(float(FSlateApplication::Get().GetCurrentTime() - StartTime));
    const float R = Size * .5f * Now.X;
    static const FSlateRoundedBoxBrush Circle = CircleBrush();
    if (Now.Z > 0.f && Now.Z < 1.f)  // 퍼지는 고리
      FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), CirclePath(Center, Size * .5f * (1.f + .3f * Now.Z)), ESlateDrawEffect::None, Color.CopyWithNewOpacity(.5f * (1.f - Now.Z)) * Tint, true, 2.f);
    if (R > 0.f) FSlateDrawElement::MakeBox(Out, LayerId + 1, Geometry.ToPaintGeometry(FVector2f(2.f * R), FSlateLayoutTransform(Center - FVector2f(R))), &Circle, ESlateDrawEffect::None, Color * Tint);
    if (Now.Y > 0.f)
      for (const TArray<FVector2f>& Stroke : DrGlyph::Strokes(EDrGlyphShape::Check)) {
        TArray<FVector2f> P;
        for (const FVector2f& Q : DrGlyph::Trim(Stroke, Now.Y)) P.Add(Center - FVector2f(Size * .32f) + Q * (Size * .64f));
        if (P.Num() > 1) FSlateDrawElement::MakeLines(Out, LayerId + 2, Geometry.ToPaintGeometry(), P, ESlateDrawEffect::None, Ink * Tint, true, FMath::Max(2.f, Size / 16.f));
      }
    return LayerId + 2;
  }
private:
  EActiveTimerReturnType Animate(double, float) {
    Invalidate(EInvalidateWidgetReason::Paint);
    if (FSlateApplication::Get().GetCurrentTime() - StartTime < 1.0) return EActiveTimerReturnType::Continue;
    Timer.Reset();
    return EActiveTimerReturnType::Stop;
  }
  TSharedPtr<FActiveTimerHandle> Timer;
};

void UDrCompletion::Play() { if (Slate) Slate->Play(IsFeedbackReduceMotion()); }
void UDrCompletion::ResetMark() { if (Slate) Slate->Reset(); }

void UDrCompletion::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Slate->Size = MarkSize;
  Slate->Color = Theme.GetColor(ColorToken);
  Slate->Ink = Theme.GetColor(InkToken);
  Slate->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UDrCompletion::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrCompletion);
  ApplyTheme();
  if (bStartComplete) Slate->Play(true);
  return Slate.ToSharedRef();
}

void UDrCompletion::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrCompletion::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 빈 화면

void UDrEmptyState::SetContent(EDrGlyphShape InShape, const FText& InTitle, const FText& InBody, const FText& InActionLabel, const FText& InSecondaryLabel) {
  Shape = InShape;
  Title = InTitle;
  Body = InBody;
  ActionLabel = InActionLabel;
  SecondaryLabel = InSecondaryLabel;
  ApplyTheme();
}

TSharedRef<SWidget> UDrEmptyState::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    WidgetTree->RootWidget = Column;
    USizeBox* CircleSize = WidgetTree->ConstructWidget<USizeBox>();
    CircleSize->SetWidthOverride(64.f);
    CircleSize->SetHeightOverride(64.f);
    IconCircle = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("IconCircle"));
    IconCircle->FillToken = DrUITokens::AccentSoft;
    IconCircle->FillStrongToken = NAME_None; IconCircle->BackdropToken = NAME_None; IconCircle->StrokeToken = NAME_None;
    IconCircle->RadiusToken = DrUITokens::RadiusPill; IconCircle->PaddingToken = NAME_None;
    IconCircle->SetHorizontalAlignment(HAlign_Center); IconCircle->SetVerticalAlignment(VAlign_Center);
    Icon = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Icon"));
    Icon->GlyphSize = 28.f;
    IconCircle->SetContent(Icon);
    CircleSize->AddChild(IconCircle);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(CircleSize)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 16)); }
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    TitleText->SetJustification(ETextJustify::Center);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(TitleText)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 6)); }
    BodyText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("BodyText"));
    BodyText->SetJustification(ETextJustify::Center);
    BodyText->SetAutoWrapText(true);
    USizeBox* BodySize = WidgetTree->ConstructWidget<USizeBox>();
    BodySize->SetMaxDesiredWidth(360.f);
    BodySize->AddChild(BodyText);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(BodySize)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 18)); }
    ActionButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("ActionButton"));
    ActionButton->OnClicked().AddWeakLambda(this, [this] { OnAction.Broadcast(); });
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(ActionButton)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 0, 0, 6)); }
    SecondaryButton = WidgetTree->ConstructWidget<UDrButton>(UDrButton::StaticClass(), TEXT("SecondaryButton"));
    SecondaryButton->Variant = EDrButtonVariant::Ghost;
    SecondaryButton->OnClicked().AddWeakLambda(this, [this] { OnSecondary.Broadcast(); });
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(SecondaryButton)) S->SetHorizontalAlignment(HAlign_Center);
    ApplyTheme();
  }
  return Super::RebuildWidget();
}

void UDrEmptyState::ApplyTheme() {
  if (IconCircle) IconCircle->ApplyTheme();
  if (Icon) { Icon->SetShape(Shape); Icon->SetColorToken(DrUITokens::Accent); }
  if (TitleText) { TitleText->SetTokens(TEXT("heading"), DrUITokens::Ink); TitleText->SetText(Title); }
  if (BodyText) { BodyText->SetTokens(DrUITokens::Body, DrUITokens::InkMuted); BodyText->SetText(Body); }
  if (ActionButton) { ActionButton->SetLabel(ActionLabel); ActionButton->SetVisibility(ActionLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible); }
  if (SecondaryButton) { SecondaryButton->SetLabel(SecondaryLabel); SecondaryButton->SetVisibility(SecondaryLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible); }
}
