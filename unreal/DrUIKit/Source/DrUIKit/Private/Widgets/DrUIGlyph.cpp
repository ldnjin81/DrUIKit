#include "Widgets/DrUIGlyph.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Rendering/DrawElements.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/SLeafWidget.h"

TArray<TArray<FVector2f>> DrUIGlyph::Strokes(EDrUIGlyphShape Shape) {
  switch (Shape) {
  case EDrUIGlyphShape::Check: return {{{.2f, .52f}, {.42f, .72f}, {.8f, .3f}}};
  case EDrUIGlyphShape::Minus: return {{{.22f, .5f}, {.78f, .5f}}};
  case EDrUIGlyphShape::Plus: return {{{.22f, .5f}, {.78f, .5f}}, {{.5f, .22f}, {.5f, .78f}}};
  case EDrUIGlyphShape::Close: return {{{.26f, .26f}, {.74f, .74f}}, {{.74f, .26f}, {.26f, .74f}}};
  case EDrUIGlyphShape::ChevronLeft: return {{{.6f, .22f}, {.34f, .5f}, {.6f, .78f}}};
  case EDrUIGlyphShape::ChevronRight: return {{{.4f, .22f}, {.66f, .5f}, {.4f, .78f}}};
  case EDrUIGlyphShape::ChevronDown: return {{{.22f, .4f}, {.5f, .66f}, {.78f, .4f}}};
  // 느낌표: 긴 세로획 + 아래 짧은 획(점).
  case EDrUIGlyphShape::Alert: return {{{.5f, .14f}, {.5f, .6f}}, {{.5f, .74f}, {.5f, .88f}}};
  default: return {};
  }
}

TArray<FVector2f> DrUIGlyph::Trim(const TArray<FVector2f>& Points, float Fraction) {
  if (Points.Num() < 2 || Fraction >= 1.f) return Points;
  if (Fraction <= 0.f) return {};
  float Total = 0.f;
  for (int32 I = 1; I < Points.Num(); ++I) Total += FVector2f::Distance(Points[I - 1], Points[I]);
  float Left = Total * Fraction;
  TArray<FVector2f> Out = {Points[0]};
  for (int32 I = 1; I < Points.Num(); ++I) {
    const float Segment = FVector2f::Distance(Points[I - 1], Points[I]);
    if (Segment >= Left) { Out.Add(FMath::Lerp(Points[I - 1], Points[I], Segment > 0.f ? Left / Segment : 0.f)); break; }
    Out.Add(Points[I]);
    Left -= Segment;
  }
  return Out;
}

class SDrUIGlyph : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrUIGlyph) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f); DotBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius; }

  EDrUIGlyphShape Shape = EDrUIGlyphShape::Check;
  FLinearColor Color = FLinearColor::White;
  float Size = 16.f, Thickness = 2.f, Progress = 1.f;

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size); }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool) const override {
    if (Progress <= 0.f) return LayerId;
    const FVector2f Local = Geometry.GetLocalSize();
    const float Side = FMath::Min(Local.X, Local.Y);
    const FVector2f Origin = (Local - FVector2f(Side)) * .5f;
    const FLinearColor Tint = Color * InWidgetStyle.GetColorAndOpacityTint();
    if (Shape == EDrUIGlyphShape::Dot) {
      const float D = Side * .5f * FMath::Clamp(Progress, 0.f, 1.2f);
      FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(FVector2f(D), FSlateLayoutTransform(Origin + FVector2f((Side - D) * .5f))), &DotBrush, ESlateDrawEffect::None, Tint);
      return LayerId;
    }
    const float Width = Thickness * Side / 16.f;
    for (const TArray<FVector2f>& Stroke : DrUIGlyph::Strokes(Shape)) {
      TArray<FVector2f> Points = DrUIGlyph::Trim(Stroke, Progress);
      if (Points.Num() < 2) continue;
      for (FVector2f& P : Points) P = Origin + P * Side;
      FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), MoveTemp(Points), ESlateDrawEffect::None, Tint, true, Width);
    }
    return LayerId;
  }
private:
  FSlateRoundedBoxBrush DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
};

void UDrUIGlyph::SetShape(EDrUIGlyphShape InShape) { Shape = InShape; ApplyTheme(); }
void UDrUIGlyph::SetColorToken(FName Token) { ColorToken = Token; ApplyTheme(); }
void UDrUIGlyph::SetProgress(float InProgress) {
  Progress = InProgress;
  if (Slate) { Slate->Progress = Progress; Slate->Invalidate(EInvalidateWidgetReason::Paint); }
}

void UDrUIGlyph::ApplyTheme() {
  if (!Slate) return;
  Slate->Shape = Shape;
  Slate->Color = UDrUIThemeSubsystem::Current().GetColor(ColorToken);
  Slate->Thickness = Thickness;
  Slate->Progress = Progress;
  if (Slate->Size != GlyphSize) { Slate->Size = GlyphSize; Slate->Invalidate(EInvalidateWidgetReason::Layout); }
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

void UDrUIGlyph::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

TSharedRef<SWidget> UDrUIGlyph::RebuildWidget() {
  SAssignNew(Slate, SDrUIGlyph);
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Slate.ToSharedRef();
}

void UDrUIGlyph::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
