#include "Widgets/DrSlider.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Rendering/DrawElements.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

/** 슬라이더 뒤 트랙: 전체 홈 + 값까지 채운 부분. */
class SDrSliderTrack : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrSliderTrack) {}
  SLATE_ATTRIBUTE(float, Fraction)
  SLATE_END_ARGS()
  void Construct(const FArguments& Args) {
    Fraction = Args._Fraction;
    Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
    Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  }
  FLinearColor TrackColor = FLinearColor::Gray, FillColor = FLinearColor::Blue;
  float Thickness = 4.f, ThumbSize = 18.f;

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(ThumbSize * 2.f, ThumbSize); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool bParentEnabled) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const float Y = (Size.Y - Thickness) * .5f;
    // 비활성이면 트랙도 흐리게(손잡이는 비활성 브러시가 흐리다).
    const FLinearColor Tint = Style.GetColorAndOpacityTint() * FLinearColor(1, 1, 1, ShouldBeEnabled(bParentEnabled) ? 1.f : .4f);
    FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(FVector2f(Size.X, Thickness), FSlateLayoutTransform(FVector2f(0.f, Y))), &Brush, ESlateDrawEffect::None, TrackColor * Tint);
    const float End = UDrSlider::FillEnd(Size.X, Fraction.Get(), ThumbSize);
    FSlateDrawElement::MakeBox(Out, LayerId + 1, Geometry.ToPaintGeometry(FVector2f(End, Thickness), FSlateLayoutTransform(FVector2f(0.f, Y))), &Brush, ESlateDrawEffect::None, FillColor * Tint);
    return LayerId + 1;
  }
private:
  TAttribute<float> Fraction;
  FSlateRoundedBoxBrush Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
};

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
    Track->Thickness = TrackThickness;
    Track->ThumbSize = ThumbSize;
    Track->Invalidate(EInvalidateWidgetReason::Paint);
  }
}

TSharedRef<SWidget> UDrSlider::RebuildWidget() {
  TSharedRef<SWidget> Slider = Super::RebuildWidget();
  SAssignNew(Track, SDrSliderTrack).Fraction(TAttribute<float>::CreateWeakLambda(this, [this] { return GetFraction(); }));
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return SNew(SOverlay) + SOverlay::Slot()[Track.ToSharedRef()] + SOverlay::Slot()[Slider];
}

void UDrSlider::SynchronizeProperties() {
  ApplyTheme();
  Super::SynchronizeProperties();
}

void UDrSlider::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Track.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
