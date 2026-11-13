#include "Widgets/DrSurface.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Core/DrUIDraw.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

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
      // CSS 그림자 근사: 그림자 사각형 = 표면을 Offset만큼 옮기고 Spread만큼 넓힌 것, 가장자리는 표준편차 Blur/2로 흐림.
      // 표면 바깥 거리 D의 진하기 = A·Φ(-(D - Spread)/σ) (정규 분포 누적, 로지스틱 근사).
      // 표면 둘레에서 바깥까지 등고선 Rings+1개를 만들고 그 사이를 정점 색이 이어지는 삼각형 띠로 채운다(줄무늬·겹침 없음, 안쪽은 비움).
      const FVector2f Offset(Shadow.Offset);
      const float Sigma = FMath::Max(Shadow.Blur * .5f, 1.f);
      const float Extent = FMath::Max(2.f, Shadow.Spread + 2.f * Sigma + Offset.Size() * .5f);
      constexpr int32 Rings = 10;
      const FSlateRenderTransform& Transform = Geometry.GetAccumulatedRenderTransform();
      TArray<FSlateVertex> Verts;
      TArray<SlateIndex> Indices;
      int32 PerLoop = 0;
      for (int32 J = 0; J <= Rings; ++J) {
        const float D = Extent * J / Rings;
        // 오프셋 방향으로 밀되 등고선이 표면 안으로 들어가지 않을 만큼만.
        const FVector2f Move = Offset.IsNearlyZero() ? FVector2f::ZeroVector : Offset.GetSafeNormal() * FMath::Min(Offset.Size(), D);
        const float Alpha = Shadow.Color.A / (1.f + FMath::Exp(1.702f * (D - Shadow.Spread) / Sigma));
        const FColor Color = (Shadow.Color.CopyWithNewOpacity(Alpha) * Tint).ToFColor(true);
        TArray<FVector2f> Loop = DrUIDraw::RoundedRectPath(Move - FVector2f(D), Size + FVector2f(2.f * D), R + D);
        Loop.Pop();  // 닫는 점(첫 점 반복)은 띠에서 쓰지 않는다
        PerLoop = Loop.Num();
        for (const FVector2f& P : Loop) Verts.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(Transform, P, FVector2f(.5f), Color));
      }
      for (int32 J = 0; J < Rings; ++J)
        for (int32 I = 0; I < PerLoop; ++I) {
          const SlateIndex A0 = J * PerLoop + I, A1 = J * PerLoop + (I + 1) % PerLoop;
          const SlateIndex B0 = A0 + PerLoop, B1 = A1 + PerLoop;
          Indices.Append({A0, A1, B1, A0, B1, B0});
        }
      static const FSlateBrush* White = FCoreStyle::Get().GetBrush("GenericWhiteBox");
      const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White);
      FSlateDrawElement::MakeCustomVerts(Out, LayerId, Handle, Verts, Indices, nullptr, 0, 0);
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
