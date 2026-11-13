#include "Widgets/DrUIStatus.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/SizeBox.h"
#include "Core/DrUIMotion.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrUISurface.h"
#include "Widgets/DrUIText.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

namespace {
bool IsReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}

/** 둥근 끝(높이 절반) 단색 브러시. */
FSlateRoundedBoxBrush PillBrush(const FLinearColor& Color, FVector2D ImageSize = FVector2D::ZeroVector) {
  FSlateRoundedBoxBrush Brush(Color, 999.f, FLinearColor::Transparent, 0.f, ImageSize);
  Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  return Brush;
}
}

// ---------------------------------------------------------------- 배지

FText UDrUIBadge::FormatCount(int32 InCount, int32 InMaxCount) {
  if (InCount <= 0) return FText::GetEmpty();
  if (InMaxCount > 0 && InCount > InMaxCount) return FText::Format(NSLOCTEXT("DrUIBadge", "Overflow", "{0}+"), FText::AsNumber(InMaxCount));
  return FText::AsNumber(InCount);
}

void UDrUIBadge::SetCount(int32 InCount) { Count = InCount; ApplyTheme(); }
void UDrUIBadge::SetDot(bool bInDot) { bDot = bInDot; ApplyTheme(); }

TSharedRef<SWidget> UDrUIBadge::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Surface = WidgetTree->ConstructWidget<UDrUISurface>(UDrUISurface::StaticClass(), TEXT("Surface"));
    Surface->RadiusToken = TEXT("radius-pill");
    Surface->PaddingToken = NAME_None;
    Surface->BackdropToken = NAME_None;
    Surface->StrokeToken = NAME_None;
    Surface->SetHorizontalAlignment(HAlign_Center);
    Surface->SetVerticalAlignment(VAlign_Center);
    CountText = WidgetTree->ConstructWidget<UDrUIText>(UDrUIText::StaticClass(), TEXT("CountText"));
    CountText->TextStyle = TEXT("caption");
    Surface->SetContent(CountText);
    Frame->AddChild(Surface);
    WidgetTree->RootWidget = Frame;
    ApplyTheme();
  }
  return Super::RebuildWidget();
}

void UDrUIBadge::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrUIBadge::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrUIBadge::ApplyTheme() {
  if (!Surface || !CountText || !Frame) return;
  FName Fill = TEXT("accent"), Ink = TEXT("on-accent");
  switch (Tone) {
  case EDrUIBadgeTone::Alert: Fill = TEXT("ember"); break;
  case EDrUIBadgeTone::Danger: Fill = TEXT("danger"); break;
  case EDrUIBadgeTone::Success: Fill = TEXT("success"); break;
  case EDrUIBadgeTone::Warning: Fill = TEXT("warning"); break;
  case EDrUIBadgeTone::Neutral: Fill = TEXT("chip"); Ink = TEXT("on-chip"); break;
  default: break;
  }
  Surface->FillToken = Fill;
  Surface->FillStrongToken = NAME_None;
  Surface->ApplyTheme();
  const FText Text = FormatCount(Count, MaxCount);
  const bool bVisible = bDot || !Text.IsEmpty();
  SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  CountText->SetVisibility(bDot ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  CountText->ColorToken = Ink;
  CountText->ApplyTheme();
  CountText->SetText(Text);
  if (bDot) {
    Frame->SetWidthOverride(8.f);
    Frame->SetHeightOverride(8.f);
    Surface->SetPadding(FMargin(0.f));
  } else {
    Frame->ClearWidthOverride();
    Frame->ClearHeightOverride();
    Frame->SetMinDesiredWidth(18.f);
    Frame->SetMinDesiredHeight(18.f);
    Surface->SetPadding(FMargin(5.f, 1.f));
  }
}

// ---------------------------------------------------------------- 진행 막대

float UDrUIProgressBar::MarqueeStart(double Seconds) {
  // 0 → .7 → 0 을 1.6초에 왕복(이즈 인·아웃 사인).
  const float Phase = float(FMath::Fmod(Seconds, 1.6) / 1.6);
  return .7f * (.5f - .5f * FMath::Cos(2.f * PI * Phase));
}

/** 끝없는 진행: 막대 위를 오가는 짧은 알약. */
class SDrUIProgressMarquee : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrUIProgressMarquee) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrUIProgressMarquee::Animate)); }
  FSlateRoundedBoxBrush Brush = PillBrush(FLinearColor::White);
  FLinearColor Color = FLinearColor::Blue;
  float Thickness = 6.f;
  /** 엔진 UProgressBar의 끝없는 진행 여부(SetIsMarquee는 다시 칠하라는 알림이 없어 매번 읽는다). */
  TAttribute<bool> Active = false;
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(0.f, Thickness); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    if (!Active.Get()) return LayerId;
    const FVector2f Size = Geometry.GetLocalSize();
    const float X = UDrUIProgressBar::MarqueeStart(FSlateApplication::Get().GetCurrentTime()) * Size.X;
    FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(FVector2f(Size.X * .3f, Size.Y), FSlateLayoutTransform(FVector2f(X, 0.f))), &Brush, ESlateDrawEffect::None, Color * Style.GetColorAndOpacityTint());
    return LayerId;
  }
private:
  EActiveTimerReturnType Animate(double, float) {
    if (Active.Get()) Invalidate(EInvalidateWidgetReason::Paint);
    return EActiveTimerReturnType::Continue;
  }
};

UDrUIProgressBar::UDrUIProgressBar() { SetBorderPadding(FVector2D::ZeroVector); }

void UDrUIProgressBar::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const FLinearColor Fill = Theme.GetColor(FillToken);
  FProgressBarStyle Style = GetWidgetStyle();
  Style.SetBackgroundImage(PillBrush(Theme.GetColor(TrackToken), FVector2D(Thickness)))
    .SetFillImage(PillBrush(Fill, FVector2D(Thickness)))
    .SetMarqueeImage(FSlateNoResource())  // 끝없는 진행은 SDrUIProgressMarquee가 그린다
    // 엔진의 채움 움직임을 쓴다. 움직임 줄이기면 바로 맞춘다.
    .SetEnableFillAnimation(!IsReduceMotion());
  SetWidgetStyle(Style);
  SetFillColorAndOpacity(FLinearColor::White);
  if (Marquee) {
    Marquee->Color = Fill;
    Marquee->Thickness = Thickness;
    Marquee->Invalidate(EInvalidateWidgetReason::Paint);
  }
}

TSharedRef<SWidget> UDrUIProgressBar::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  TSharedRef<SWidget> Bar = Super::RebuildWidget();
  SAssignNew(Marquee, SDrUIProgressMarquee);
  Marquee->Active = TAttribute<bool>::CreateWeakLambda(this, [this] { return UseMarquee(); });
  ApplyTheme();
  return SNew(SOverlay) + SOverlay::Slot()[Bar] + SOverlay::Slot().VAlign(VAlign_Center)[Marquee.ToSharedRef()];
}

void UDrUIProgressBar::SynchronizeProperties() {
  Super::SynchronizeProperties();
  ApplyTheme();
}

void UDrUIProgressBar::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Marquee.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 진행 링

TArray<FVector2f> DrUIProgressRing::ArcPoints(FVector2f Center, float Radius, float Start, float Sweep, int32 Segments) {
  TArray<FVector2f> Points;
  if (Sweep <= 0.f || Segments <= 0) return Points;
  const int32 Count = FMath::Max(1, FMath::CeilToInt(Segments * FMath::Min(Sweep, 1.f)));
  for (int32 I = 0; I <= Count; ++I) {
    // 0 = 12시, 화면 y가 아래로 커지므로 각이 커지면 시계 방향.
    const float Angle = 2.f * PI * (Start + FMath::Min(Sweep, 1.f) * I / Count) - .5f * PI;
    Points.Add(Center + Radius * FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)));
  }
  return Points;
}

class SDrUIProgressRing : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrUIProgressRing) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) {
    Shown.Snap(0.f);
    RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrUIProgressRing::Animate));
  }
  float Size = 40.f, Thickness = 4.f;
  FLinearColor TrackColor = FLinearColor::Gray, FillColor = FLinearColor::Blue;
  bool bIndeterminate = false;
  FDrUISpring Shown;

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Local = Geometry.GetLocalSize();
    const float Radius = (FMath::Min(Local.X, Local.Y) - Thickness) * .5f;
    const FVector2f Center = Local * .5f;
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), DrUIProgressRing::ArcPoints(Center, Radius, 0.f, 1.f), ESlateDrawEffect::None, TrackColor * Tint, true, Thickness);
    float Start = 0.f, Sweep = FMath::Clamp(Shown.Value, 0.f, 1.f);
    if (bIndeterminate) {
      // 1초에 한 바퀴, 호 길이 1/4.
      Start = FMath::Fractional(float(FSlateApplication::Get().GetCurrentTime()));
      Sweep = .25f;
    }
    if (Sweep > 0.f)
      FSlateDrawElement::MakeLines(Out, LayerId + 1, Geometry.ToPaintGeometry(), DrUIProgressRing::ArcPoints(Center, Radius, Start, Sweep), ESlateDrawEffect::None, FillColor * Tint, true, Thickness);
    return LayerId + 1;
  }
private:
  EActiveTimerReturnType Animate(double, float DeltaTime) {
    if (bIndeterminate || !Shown.IsSettled()) {
      Shown.Step(DeltaTime);
      Invalidate(EInvalidateWidgetReason::Paint);
    }
    return EActiveTimerReturnType::Continue;
  }
};

void UDrUIProgressRing::SetPercent(float InPercent) {
  Percent = FMath::Clamp(InPercent, 0.f, 1.f);
  if (!Slate) return;
  if (IsReduceMotion()) Slate->Shown.Snap(Percent); else Slate->Shown.Target = Percent;
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrUIProgressRing::SetIndeterminate(bool bInIndeterminate) { bIndeterminate = bInIndeterminate; ApplyTheme(); }

void UDrUIProgressRing::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Slate->TrackColor = Theme.GetColor(TrackToken);
  Slate->FillColor = Theme.GetColor(FillToken);
  Slate->Thickness = Thickness;
  Slate->bIndeterminate = bIndeterminate;
  if (Slate->Size != RingSize) { Slate->Size = RingSize; Slate->Invalidate(EInvalidateWidgetReason::Layout); }
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrUIProgressRing::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

TSharedRef<SWidget> UDrUIProgressRing::RebuildWidget() {
  SAssignNew(Slate, SDrUIProgressRing);
  Slate->Shown.Snap(Percent);  // 처음 보일 때는 차오르지 않고 그 값으로.
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Slate.ToSharedRef();
}

void UDrUIProgressRing::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 스켈레톤

float UDrUISkeleton::PulseAlpha(double Seconds) { return .25f + .07f * FMath::Sin(float(2.0 * PI * FMath::Fmod(Seconds, 1.2) / 1.2)); }

class SDrUISkeleton : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrUISkeleton) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrUISkeleton::Animate)); }
  FVector2D Size = FVector2D(160.f, 12.f);
  FLinearColor Color = FLinearColor::Gray;
  FSlateRoundedBoxBrush Brush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f);
  bool bStill = false;

  virtual FVector2D ComputeDesiredSize(float) const override { return Size; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const float Alpha = bStill ? .25f : UDrUISkeleton::PulseAlpha(FSlateApplication::Get().GetCurrentTime());
    FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(), &Brush, ESlateDrawEffect::None, Color.CopyWithNewOpacity(Color.A * Alpha) * Style.GetColorAndOpacityTint());
    return LayerId;
  }
private:
  EActiveTimerReturnType Animate(double, float) {
    if (!bStill) Invalidate(EInvalidateWidgetReason::Paint);
    return EActiveTimerReturnType::Continue;
  }
};

void UDrUISkeleton::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const float Radius = Theme.GetRadius(RadiusToken);
  Slate->Brush = FSlateRoundedBoxBrush(FLinearColor::White, Radius);
  if (Radius >= 500.f) Slate->Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  Slate->Color = Theme.GetColor(ColorToken);
  Slate->bStill = IsReduceMotion();
  if (Slate->Size != BlockSize) { Slate->Size = BlockSize; Slate->Invalidate(EInvalidateWidgetReason::Layout); }
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrUISkeleton::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

TSharedRef<SWidget> UDrUISkeleton::RebuildWidget() {
  SAssignNew(Slate, SDrUISkeleton);
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Slate.ToSharedRef();
}

void UDrUISkeleton::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 스피너

UDrUISpinner::UDrUISpinner(const FObjectInitializer& Initializer) : Super(Initializer) {
  SetNumberOfPieces(8);
  SetRadius(8.f);
}

void UDrUISpinner::ApplyTheme() {
  FSlateBrush Piece = GetImage();
  Piece.TintColor = FSlateColor(UDrUIThemeSubsystem::Current().GetColor(ColorToken));
  Piece.ImageSize = FVector2D(PieceSize > 0.f ? PieceSize : AutoPieceSize(GetRadius()));  // 기본 조각은 커서 작은 반경에서 서로 겹쳐 덩어리로 보인다.
  SetImage(Piece);
}

TSharedRef<SWidget> UDrUISpinner::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Super::RebuildWidget();
}

void UDrUISpinner::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

void UDrUISpinner::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
