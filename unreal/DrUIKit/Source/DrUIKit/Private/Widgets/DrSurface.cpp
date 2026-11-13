#include "Widgets/DrSurface.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Rendering/DrawElements.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

/** 둥근 사각형 둘레(닫힌 꺾은선). 모서리마다 8조각. */
static TArray<FVector2f> RoundedRectPath(FVector2f Min, FVector2f Size, float Radius) {
  const float R = FMath::Clamp(Radius, 0.f, FMath::Min(Size.X, Size.Y) * .5f);
  const FVector2f Centers[4] = {Min + FVector2f(Size.X - R, R), Min + FVector2f(Size.X - R, Size.Y - R), Min + FVector2f(R, Size.Y - R), Min + FVector2f(R)};
  TArray<FVector2f> Points;
  for (int32 Corner = 0; Corner < 4; ++Corner)
    for (int32 I = 0; I <= 8; ++I) {
      // 오른쪽 위 모서리부터 시계 방향(-90° → 0° → 90° …).
      const float Angle = (-90.f + 90.f * Corner + 90.f * I / 8.f) * PI / 180.f;
      Points.Add(Centers[Corner] + R * FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)));
    }
  const FVector2f First = Points[0];  // 자기 원소를 그대로 Add하면 재할당 중 참조가 깨진다(TArray 어서트)
  Points.Add(First);
  return Points;
}

/** 표면 장식: 뒤 그림자(표면 바깥 고리 겹) 또는 앞 윗면 반사광(윗변 안쪽 1px 선). 눌림 판정 없음. */
class SDrSurfaceDecor : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrSurfaceDecor) {}
  SLATE_ARGUMENT(bool, bShadow)
  SLATE_END_ARGS()
  void Construct(const FArguments& Args) { bShadow = Args._bShadow; SetVisibility(EVisibility::HitTestInvisible); }
  FDrUIShadow Shadow;
  FLinearColor HighlightColor = FLinearColor::Transparent;
  float Radius = 0.f;
  bool bPill = false;

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const float R = bPill ? Size.Y * .5f : FMath::Min(Radius, FMath::Min(Size.X, Size.Y) * .5f);
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    if (bShadow) {
      if (Shadow.Color.A <= 0.f) return LayerId;
      // 그림자가 표면 밖으로 번지는 거리(흐림 절반 + 퍼짐 + 오프셋). 고리 Rings개로 나눠 바깥일수록 옅게.
      const FVector2f Offset(Shadow.Offset);
      const float Extent = FMath::Max(2.f, Shadow.Blur * .5f + Shadow.Spread + Offset.Size());
      constexpr int32 Rings = 8;
      const float Width = Extent / Rings;
      for (int32 K = 0; K < Rings; ++K) {
        const float D = (K + .5f) * Width;  // 고리 띠의 가운데 거리
        // 고리가 표면 안으로 들어가지 않을 만큼만 오프셋 방향으로 민다(바깥 띠만 그린다).
        const float Shift = FMath::Min(Offset.Size(), FMath::Max(0.f, D - Width * .5f));
        const FVector2f Move = Offset.IsNearlyZero() ? FVector2f::ZeroVector : Offset.GetSafeNormal() * Shift;
        const float Fade = FMath::Square(1.f - (K + .5f) / Rings);
        // 둥근 상자 브러시는 채움을 그릴 때 넘기는 색으로 칠해 고리만 그릴 수 없다. 둘레를 굵은 선으로 따라간다.
        const TArray<FVector2f> Path = RoundedRectPath(Move - FVector2f(D), Size + FVector2f(2.f * D), R + D);
        FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), Path, ESlateDrawEffect::None, Shadow.Color.CopyWithNewOpacity(Shadow.Color.A * Fade * 1.6f) * Tint, true, Width);
      }
      return LayerId;
    }
    if (HighlightColor.A <= 0.f || Size.X <= 2.f * R) return LayerId;
    // 윗변의 곧은 부분에만 1px 선(모서리 곡선 앞에서 멈춤).
    const float Inset = FMath::Max(R * .7f, 2.f);
    const TArray<FVector2f> Line = {FVector2f(Inset, .75f), FVector2f(Size.X - Inset, .75f)};
    FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, HighlightColor * Tint, true, 1.f);
    return LayerId;
  }
private:
  bool bShadow = true;
};

UDrSurface::UDrSurface() { SetHorizontalAlignment(HAlign_Fill); SetVerticalAlignment(VAlign_Fill); }

static EDrUISurfaceQuality CurrentQuality() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem ? Subsystem->GetSurfaceQuality() : GetDefault<UDrUIKitSettings>()->SurfaceQuality;
}

FLinearColor UDrSurface::ResolveFill() const {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const bool bNoBlur = BackdropToken.IsNone() || CurrentQuality() == EDrUISurfaceQuality::Low;
  const FName Token = bNoBlur && !FillStrongToken.IsNone() ? FillStrongToken : FillToken;
  if (Token.IsNone()) return FLinearColor::Transparent;
  FLinearColor Color = Theme.GetColor(Token);
  if (Theme.TranslucentTokens.Contains(Token)) {
    const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
    Color.A = UDrUITheme::AdjustAlpha(Color.A, Subsystem ? Subsystem->GetSurfaceTransparency() : GetDefault<UDrUIKitSettings>()->SurfaceTransparency);
  }
  return Color;
}

float UDrSurface::ResolveBlur() const {
  if (BackdropToken.IsNone()) return 0.f;
  const EDrUISurfaceQuality Quality = CurrentQuality();
  if (Quality == EDrUISurfaceQuality::Low) return 0.f;
  const float Strength = UDrUIThemeSubsystem::Current().GetBackdrop(BackdropToken);
  return Quality == EDrUISurfaceQuality::Medium ? Strength * .5f : Strength;
}

void UDrSurface::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const float Radius = RadiusToken.IsNone() ? 0.f : Theme.GetRadius(RadiusToken);
  const FLinearColor Stroke = StrokeToken.IsNone() ? FLinearColor::Transparent : Theme.GetColor(StrokeToken);
  FSlateRoundedBoxBrush Brush(ResolveFill(), Radius, Stroke, StrokeToken.IsNone() ? 0.f : StrokeWidth);
  // 알약(radius-pill = 999)은 반경을 그대로 쓰면 마름모처럼 그려진다. 높이의 절반으로 둥글린다.
  const bool bPill = Radius >= 500.f;
  if (bPill) Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  SetBrush(Brush);
  if (!PaddingToken.IsNone()) SetPadding(FMargin(Theme.GetSpacing(PaddingToken)));
  if (Shadow) {
    Shadow->Shadow = ShadowToken.IsNone() ? FDrUIShadow() : Theme.GetShadow(ShadowToken);
    Shadow->Radius = Radius;
    Shadow->bPill = bPill;
    Shadow->Invalidate(EInvalidateWidgetReason::Paint);
  }
  if (Highlight) {
    Highlight->HighlightColor = HighlightToken.IsNone() ? FLinearColor::Transparent : Theme.GetColor(HighlightToken);
    Highlight->Radius = Radius;
    Highlight->bPill = bPill;
    Highlight->Invalidate(EInvalidateWidgetReason::Paint);
  }
  if (Blur) {
    Blur->SetBlurStrength(ResolveBlur());
    // 흐림은 반경을 그대로 받으므로 알약은 적당히 크게(대부분의 바 높이의 절반 이상)만 준다.
    const float BlurRadius = bPill ? 24.f : Radius;
    Blur->SetCornerRadius(FVector4(BlurRadius, BlurRadius, BlurRadius, BlurRadius));
  }
}

void UDrSurface::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrSurface::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  TSharedRef<SWidget> Inner = Super::RebuildWidget();
  // 흐림 강도 0이면 SBackgroundBlur는 그리지 않는다. 품질이 바뀌어도 트리를 다시 만들 필요가 없게 늘 감싼다.
  SAssignNew(Blur, SBackgroundBlur).BlurStrength(0.f).bApplyAlphaToBlur(true).Padding(0.f)[Inner];
  // 뒤 그림자 · 흐림+표면 · 앞 반사광(내용 위에 1px). 장식이 없으면 그리지 않는다.
  SAssignNew(Shadow, SDrSurfaceDecor).bShadow(true);
  SAssignNew(Highlight, SDrSurfaceDecor).bShadow(false);
  ApplyTheme();
  return SNew(SOverlay) + SOverlay::Slot()[Shadow.ToSharedRef()] + SOverlay::Slot()[Blur.ToSharedRef()] + SOverlay::Slot()[Highlight.ToSharedRef()];
}

void UDrSurface::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Blur.Reset();
  Shadow.Reset();
  Highlight.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
