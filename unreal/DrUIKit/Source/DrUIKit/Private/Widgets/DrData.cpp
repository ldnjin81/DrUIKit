#include "Widgets/DrData.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/DrUIDraw.h"
#include "Fonts/FontMeasure.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrCountUp.h"
#include "Widgets/DrStatus.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"
#include "Widgets/SLeafWidget.h"

// ---------------------------------------------------------------- 스파크라인

TArray<FVector2f> UDrSparkline::ToPoints(const TArray<float>& InValues, FVector2f Size, float Pad) {
  TArray<FVector2f> Points;
  if (InValues.Num() < 2) return Points;
  float Min = InValues[0], Max = InValues[0];
  for (const float V : InValues) { Min = FMath::Min(Min, V); Max = FMath::Max(Max, V); }
  const float Range = Max - Min;
  for (int32 I = 0; I < InValues.Num(); ++I) {
    const float X = Size.X * I / (InValues.Num() - 1);
    const float T = Range > 0.f ? (InValues[I] - Min) / Range : .5f;  // 모두 같으면 가운데 수평선
    Points.Add(FVector2f(X, Pad + (1.f - T) * (Size.Y - 2.f * Pad)));
  }
  return Points;
}

class SDrSparkline : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrSparkline) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { Dot = FSlateRoundedBoxBrush(FLinearColor::White, 999.f); Dot.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius; }
  TArray<float> Values;
  FVector2D DesiredSize = FVector2D(140.f, 36.f);
  FLinearColor Line = FLinearColor::Blue;
  virtual FVector2D ComputeDesiredSize(float) const override { return DesiredSize; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const TArray<FVector2f> Points = UDrSparkline::ToPoints(Values, Size);
    if (Points.Num() < 2) return LayerId;
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    // 아래 채움: 선 아래를 위 25% → 바닥 0%로 옅게(꺾은선과 바닥 사이 사다리꼴 띠).
    TArray<FVector2f> P;
    TArray<FLinearColor> C;
    TArray<SlateIndex> I;
    for (int32 K = 0; K < Points.Num(); ++K) {
      P.Add(Points[K]); C.Add(Line.CopyWithNewOpacity(.22f) * Tint);
      P.Add(FVector2f(Points[K].X, Size.Y)); C.Add(Line.CopyWithNewOpacity(0.f) * Tint);
      if (K > 0) { const SlateIndex B = (K - 1) * 2; I.Append({B, SlateIndex(B + 2), SlateIndex(B + 3), B, SlateIndex(B + 3), SlateIndex(B + 1)}); }
    }
    DrUIDraw::Triangles(Out, LayerId, Geometry, P, C, I);
    FSlateDrawElement::MakeLines(Out, LayerId + 1, Geometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Line * Tint, true, 2.f);
    const FVector2f Last = Points.Last();
    FSlateDrawElement::MakeBox(Out, LayerId + 2, Geometry.ToPaintGeometry(FVector2f(6.f), FSlateLayoutTransform(Last - FVector2f(3.f))), &Dot, ESlateDrawEffect::None, Line * Tint);
    return LayerId + 2;
  }
private:
  FSlateRoundedBoxBrush Dot = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
};

void UDrSparkline::SetValues(const TArray<float>& InValues) { Values = InValues; ApplyTheme(); }

void UDrSparkline::ApplyTheme() {
  if (!Slate) return;
  Slate->Values = Values;
  Slate->DesiredSize = LineSize;
  Slate->Line = UDrUIThemeSubsystem::Current().GetColor(LineToken);
  Slate->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UDrSparkline::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrSparkline);
  ApplyTheme();
  return Slate.ToSharedRef();
}

void UDrSparkline::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrSparkline::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 도넛

TArray<FVector2f> UDrDonut::Segments(const TArray<float>& InValues) {
  TArray<FVector2f> Out;
  float Sum = 0.f;
  for (const float V : InValues) Sum += FMath::Max(0.f, V);
  if (Sum <= 0.f) return Out;
  float Start = 0.f;
  for (const float V : InValues) {
    const float Length = FMath::Max(0.f, V) / Sum;
    Out.Add(FVector2f(Start, Length));
    Start += Length;
  }
  return Out;
}

class SDrDonut : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrDonut) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) {}
  TArray<float> Values;
  TArray<FLinearColor> Colors;
  FText CenterValue, CenterCaption;
  FSlateFontInfo ValueFont, CaptionFont;
  FLinearColor Ink = FLinearColor::Black, Muted = FLinearColor::Gray, Track = FLinearColor::Gray;
  float Size = 120.f, Thickness = 14.f;
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Local = Geometry.GetLocalSize();
    const FVector2f Center = Local * .5f;
    const float R = (FMath::Min(Local.X, Local.Y) - Thickness) * .5f;
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), DrProgressRing::ArcPoints(Center, R, 0.f, 1.f), ESlateDrawEffect::None, Track * Tint, true, Thickness);
    const TArray<FVector2f> Segs = UDrDonut::Segments(Values);
    // 조각 사이 틈(둘레 기준 약 3px).
    const float Gap = Segs.Num() > 1 ? 3.f / (2.f * PI * R) : 0.f;
    for (int32 K = 0; K < Segs.Num(); ++K) {
      const float Length = Segs[K].Y - Gap;
      if (Length <= 0.f || Colors.IsEmpty()) continue;
      FSlateDrawElement::MakeLines(Out, LayerId + 1, Geometry.ToPaintGeometry(), DrProgressRing::ArcPoints(Center, R, Segs[K].X + Gap * .5f, Length, 160), ESlateDrawEffect::None, Colors[K % Colors.Num()] * Tint, true, Thickness);
    }
    const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FVector2D ValueSize = Measure->Measure(CenterValue, ValueFont), CaptionSize = Measure->Measure(CenterCaption, CaptionFont);
    const float Top = Center.Y - float(ValueSize.Y + CaptionSize.Y) * .5f;
    FSlateDrawElement::MakeText(Out, LayerId + 2, Geometry.ToPaintGeometry(FVector2f(ValueSize), FSlateLayoutTransform(FVector2f(Center.X - float(ValueSize.X) * .5f, Top))), CenterValue, ValueFont, ESlateDrawEffect::None, Ink * Tint);
    FSlateDrawElement::MakeText(Out, LayerId + 2, Geometry.ToPaintGeometry(FVector2f(CaptionSize), FSlateLayoutTransform(FVector2f(Center.X - float(CaptionSize.X) * .5f, Top + float(ValueSize.Y)))), CenterCaption, CaptionFont, ESlateDrawEffect::None, Muted * Tint);
    return LayerId + 2;
  }
};

void UDrDonut::SetData(const TArray<float>& InValues, const FText& InCenterValue, const FText& InCenterCaption) {
  Values = InValues;
  CenterValue = InCenterValue;
  CenterCaption = InCenterCaption;
  ApplyTheme();
}

void UDrDonut::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Slate->Values = Values;
  Slate->Colors.Reset();
  for (const FName Token : ColorTokens) Slate->Colors.Add(Theme.GetColor(Token).CopyWithNewOpacity(1.f));  // 굵은 원호는 이음매가 겹치므로 불투명
  Slate->CenterValue = CenterValue;
  Slate->CenterCaption = CenterCaption;
  Slate->ValueFont = Theme.MakeFont(TEXT("title"));
  Slate->CaptionFont = Theme.MakeFont(DrUITokens::Caption);
  Slate->Ink = Theme.GetColor(DrUITokens::Ink);
  Slate->Muted = Theme.GetColor(DrUITokens::InkMuted);
  Slate->Track = FMath::Lerp(Theme.GetColor(DrUITokens::GlassStrong).CopyWithNewOpacity(1.f), Theme.GetColor(DrUITokens::Ink), .08f).CopyWithNewOpacity(1.f);
  Slate->Size = DonutSize;
  Slate->Thickness = Thickness;
  Slate->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UDrDonut::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrDonut);
  ApplyTheme();
  return Slate.ToSharedRef();
}

void UDrDonut::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrDonut::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}

// ---------------------------------------------------------------- 통계 카드

FText UDrStatCard::FormatDelta(float DeltaPercent) {
  if (FMath::IsNearlyZero(DeltaPercent, .0005f)) return FText::FromString(TEXT("—"));
  FNumberFormattingOptions Format;
  Format.SetMaximumFractionalDigits(1);
  return FText::Format(NSLOCTEXT("DrStatCard", "Delta", "{0}{1}"), FText::FromString(DeltaPercent > 0.f ? TEXT("↑") : TEXT("↓")), FText::AsPercent(FMath::Abs(DeltaPercent), &Format));
}

void UDrStatCard::EnsureTree() {
  if (!WidgetTree || WidgetTree->RootWidget) return;
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = DrUITokens::GlassStrong;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusLg;
  Surface->PaddingToken = NAME_None;
  Surface->SetPadding(FMargin(16.f, 14.f));
  WidgetTree->RootWidget = Surface;
  UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
  Surface->SetContent(Column);
  LabelText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("LabelText"));
  LabelText->SetTokens(TEXT("overline"), DrUITokens::InkMuted);
  Column->AddChildToVerticalBox(LabelText);
  UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
  ValueText = WidgetTree->ConstructWidget<UDrCountUp>(UDrCountUp::StaticClass(), TEXT("ValueText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ValueText)) S->SetVerticalAlignment(VAlign_Bottom);
  DeltaText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("DeltaText"));
  if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(DeltaText)) { S->SetVerticalAlignment(VAlign_Bottom); S->SetPadding(FMargin(8, 0, 0, 5)); }
  if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Row)) S->SetPadding(FMargin(0, 4, 0, 8));
  Spark = WidgetTree->ConstructWidget<UDrSparkline>(UDrSparkline::StaticClass(), TEXT("Spark"));
  Column->AddChildToVerticalBox(Spark);
}

TSharedRef<SWidget> UDrStatCard::RebuildWidget() {
  EnsureTree();
  return Super::RebuildWidget();
}

void UDrStatCard::SetData(const FText& InLabel, float InValue, float InDeltaPercent, const TArray<float>& InHistory, ECommonNumericType InNumericType) {
  EnsureTree();
  if (LabelText) LabelText->SetText(InLabel);
  if (ValueText) { ValueText->SetNumericType(InNumericType); ValueText->CountTo(InValue); }
  if (DeltaText) {
    // 오르면 success, 내리면 danger(화살표도 함께 — 색만으로 구분하지 않음).
    DeltaText->SetTokens(DrUITokens::Label, InDeltaPercent > 0.f ? DrUITokens::Success : InDeltaPercent < 0.f ? DrUITokens::Danger : DrUITokens::InkMuted);
    DeltaText->SetText(FormatDelta(InDeltaPercent));
  }
  if (Spark) {
    Spark->LineToken = InDeltaPercent < 0.f ? DrUITokens::Danger : DrUITokens::Accent;
    Spark->SetValues(InHistory);
  }
}
