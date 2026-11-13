#include "Widgets/DrSlider.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Rendering/DrawElements.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

/** 슬라이더 뒤 트랙: 전체 홈 + 값까지 채운 부분 + 눈금 점 + 값 말풍선(손잡이 위, 위젯 밖으로 그림). */
class SDrSliderTrack : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrSliderTrack) {}
  SLATE_ATTRIBUTE(float, Fraction)
  SLATE_ATTRIBUTE(bool, ShowBubble)
  SLATE_ATTRIBUTE(FText, BubbleText)
  SLATE_END_ARGS()
  void Construct(const FArguments& Args) {
    Fraction = Args._Fraction;
    ShowBubble = Args._ShowBubble;
    BubbleText = Args._BubbleText;
    Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
    Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  }
  FLinearColor TrackColor = FLinearColor::Gray, FillColor = FLinearColor::Blue, TickColor = FLinearColor::Gray, TickOnFillColor = FLinearColor::White;
  FLinearColor BubbleFill = FLinearColor::Black, BubbleInk = FLinearColor::White;
  FSlateFontInfo BubbleFont;
  float Thickness = 4.f, ThumbSize = 18.f;
  int32 Ticks = 0;

  /** 말풍선이 시간이 지나 사라질 때까지 다시 그리게 한다. */
  void KeepPainting() {
    if (!PaintTimer.IsValid()) PaintTimer = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrSliderTrack::Repaint));
  }

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(ThumbSize * 2.f, ThumbSize); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool bParentEnabled) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const float Y = (Size.Y - Thickness) * .5f;
    // 비활성이면 트랙도 흐리게(손잡이는 비활성 브러시가 흐리다).
    const FLinearColor Tint = Style.GetColorAndOpacityTint() * FLinearColor(1, 1, 1, ShouldBeEnabled(bParentEnabled) ? 1.f : .4f);
    FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(FVector2f(Size.X, Thickness), FSlateLayoutTransform(FVector2f(0.f, Y))), &Brush, ESlateDrawEffect::None, TrackColor * Tint);
    const float Value = Fraction.Get();
    const float End = UDrSlider::FillEnd(Size.X, Value, ThumbSize);
    FSlateDrawElement::MakeBox(Out, LayerId + 1, Geometry.ToPaintGeometry(FVector2f(End, Thickness), FSlateLayoutTransform(FVector2f(0.f, Y))), &Brush, ESlateDrawEffect::None, FillColor * Tint);
    // 눈금: 채운 부분 위는 밝은 점, 나머지는 옅은 점.
    if (Ticks > 0) {
      const float Dot = FMath::Max(2.f, Thickness - 1.f);
      for (int32 I = 0; I <= Ticks; ++I) {
        const float X = UDrSlider::FillEnd(Size.X, float(I) / Ticks, ThumbSize);
        const FLinearColor Color = X <= End + .5f ? TickOnFillColor.CopyWithNewOpacity(.75f) : TickColor;
        FSlateDrawElement::MakeBox(Out, LayerId + 2, Geometry.ToPaintGeometry(FVector2f(Dot), FSlateLayoutTransform(FVector2f(X - Dot * .5f, (Size.Y - Dot) * .5f))), &Brush, ESlateDrawEffect::None, Color * Tint);
      }
    }
    // 값 말풍선: 손잡이 가운데 위(손잡이 꼭대기에서 6px 위).
    if (ShowBubble.Get()) {
      const FText Text = BubbleText.Get();
      const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text, BubbleFont);
      const FVector2f Pad(8.f, 3.f);
      const FVector2f BoxSize = FVector2f(TextSize) + Pad * 2.f;
      const FVector2f BoxPos(End - BoxSize.X * .5f, (Size.Y - ThumbSize) * .5f - 6.f - BoxSize.Y);
      FSlateDrawElement::MakeBox(Out, LayerId + 3, Geometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(BoxPos)), &Brush, ESlateDrawEffect::None, BubbleFill * Tint);
      FSlateDrawElement::MakeText(Out, LayerId + 4, Geometry.ToPaintGeometry(FVector2f(TextSize), FSlateLayoutTransform(BoxPos + Pad)), Text, BubbleFont, ESlateDrawEffect::None, BubbleInk * Tint);
      return LayerId + 4;
    }
    return LayerId + 2;
  }
private:
  EActiveTimerReturnType Repaint(double, float) {
    Invalidate(EInvalidateWidgetReason::Paint);
    if (ShowBubble.Get()) return EActiveTimerReturnType::Continue;
    PaintTimer.Reset();
    return EActiveTimerReturnType::Stop;
  }
  TAttribute<float> Fraction;
  TAttribute<bool> ShowBubble;
  TAttribute<FText> BubbleText;
  TSharedPtr<FActiveTimerHandle> PaintTimer;
  FSlateRoundedBoxBrush Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
};

int32 UDrSlider::TickSteps(float Min, float Max, float Step, int32 MaxSteps) {
  if (Step <= 0.f || Max <= Min) return 0;
  const float Steps = (Max - Min) / Step;
  const int32 Rounded = FMath::RoundToInt(Steps);
  // 단계가 범위를 딱 나누고(끝이 맞고) 너무 촘촘하지 않을 때만.
  return Rounded >= 1 && Rounded <= MaxSteps && FMath::IsNearlyEqual(Steps, float(Rounded), 1e-3f) ? Rounded : 0;
}

FText UDrSlider::GetBubbleText() const {
  const float Min = GetMinValue(), Max = GetMaxValue();
  if (FMath::IsNearlyZero(Min) && FMath::IsNearlyEqual(Max, 1.f)) return FText::AsPercent(GetValue());
  FNumberFormattingOptions Format;
  const float Step = GetStepSize();
  const int32 Digits = Step >= 1.f || Step <= 0.f ? 0 : Step >= .1f ? 1 : 2;
  Format.SetMinimumFractionalDigits(Digits).SetMaximumFractionalDigits(Digits);
  return FText::AsNumber(GetValue(), &Format);
}

bool UDrSlider::IsBubbleVisible() const {
  return bShowValueBubble && (bCaptured || FSlateApplication::Get().GetCurrentTime() < BubbleUntil);
}

void UDrSlider::HandleCaptureBegin() { bCaptured = true; if (Track) Track->KeepPainting(); }
void UDrSlider::HandleCaptureEnd() { bCaptured = false; BubbleUntil = FSlateApplication::Get().GetCurrentTime() + .4; if (Track) Track->KeepPainting(); }

void UDrSlider::HandleValueChangedForBubble(float) {
  // 키보드·패드로 바꿀 때는 잡기 이벤트가 없으므로 바꾼 직후 잠깐 보인다.
  if (!bCaptured) BubbleUntil = FSlateApplication::Get().GetCurrentTime() + .8;
  if (Track) Track->KeepPainting();
}

UDrSlider::UDrSlider() {
  // 키보드·패드 한 칸: 범위의 5%.
  SetStepSize(.05f);
}

float UDrSlider::GetFraction() const {
  const float Min = GetMinValue(), Max = GetMaxValue();
  return Max > Min ? FMath::Clamp((GetValue() - Min) / (Max - Min), 0.f, 1.f) : 0.f;
}

void UDrSlider::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const FLinearColor Fill = Theme.GetColor(FillToken), Thumb = Theme.GetColor(ThumbToken);
  // 손잡이: 채움 + accent 테두리(밝은 카드 위 흰 손잡이도 보이게). 호버는 테두리를 굵게.
  auto ThumbBrush = [&](float Outline, float Alpha) {
    FSlateRoundedBoxBrush B(Thumb.CopyWithNewOpacity(Thumb.A * Alpha), 999.f, Fill.CopyWithNewOpacity(Alpha), Outline, FVector2D(ThumbSize));
    B.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    return B;
  };
  FSliderStyle Style = GetWidgetStyle();
  FSlateBrush None;
  None.DrawAs = ESlateBrushDrawType::NoDrawType;
  Style.SetNormalBarImage(None).SetHoveredBarImage(None).SetDisabledBarImage(None).SetBarThickness(0.f);
  Style.SetNormalThumbImage(ThumbBrush(2.f, 1.f)).SetHoveredThumbImage(ThumbBrush(3.f, 1.f)).SetDisabledThumbImage(ThumbBrush(2.f, .4f));
  SetWidgetStyle(Style);
  if (Track) {
    Track->TrackColor = Theme.GetColor(TrackToken);
    Track->FillColor = Fill;
    Track->TickColor = Theme.GetColor(DrUITokens::InkFaint);
    Track->TickOnFillColor = Thumb;
    Track->BubbleFill = Theme.GetColor(DrUITokens::Chip);
    Track->BubbleInk = Theme.GetColor(DrUITokens::OnChip);
    Track->BubbleFont = Theme.MakeFont(DrUITokens::Caption);
    Track->Ticks = bShowTicks ? TickSteps(GetMinValue(), GetMaxValue(), GetStepSize(), MaxTicks) : 0;
    Track->Thickness = TrackThickness;
    Track->ThumbSize = ThumbSize;
    Track->Invalidate(EInvalidateWidgetReason::Paint);
  }
}

TSharedRef<SWidget> UDrSlider::RebuildWidget() {
  TSharedRef<SWidget> Slider = Super::RebuildWidget();
  SAssignNew(Track, SDrSliderTrack)
    .Fraction(TAttribute<float>::CreateWeakLambda(this, [this] { return GetFraction(); }))
    .ShowBubble(TAttribute<bool>::CreateWeakLambda(this, [this] { return IsBubbleVisible(); }))
    .BubbleText(TAttribute<FText>::CreateWeakLambda(this, [this] { return GetBubbleText(); }));
  OnMouseCaptureBegin.AddUniqueDynamic(this, &ThisClass::HandleCaptureBegin);
  OnMouseCaptureEnd.AddUniqueDynamic(this, &ThisClass::HandleCaptureEnd);
  OnControllerCaptureBegin.AddUniqueDynamic(this, &ThisClass::HandleCaptureBegin);
  OnControllerCaptureEnd.AddUniqueDynamic(this, &ThisClass::HandleCaptureEnd);
  OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleValueChangedForBubble);
  ApplyTheme();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return SNew(SOverlay) + SOverlay::Slot()[Track.ToSharedRef()] + SOverlay::Slot()[Slider];
}

void UDrSlider::SynchronizeProperties() {
  // 엔진이 단계·범위를 Slate 슬라이더에 넘긴 뒤에 칠한다(GetStepSize 등이 Slate 값을 읽으므로 먼저 칠하면 눈금이 옛 단계로 계산됨).
  Super::SynchronizeProperties();
  ApplyTheme();
}

void UDrSlider::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Track.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
