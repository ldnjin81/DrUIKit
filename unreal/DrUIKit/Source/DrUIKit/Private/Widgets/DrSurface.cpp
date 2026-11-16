#include "Widgets/DrSurface.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Core/DrUIDraw.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Components/BorderSlot.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

/**
 * 표면 뒤 그림자. 토큰의 바깥 겹마다 그린다(뉴모피즘은 밝은 겹 + 어두운 겹). 눌림 판정 없음.
 * 흐린 겹은 표면 바깥에만 고리 겹으로(반투명 유리 안쪽이 탁해지지 않게), 흐림 없는 겹(브루탈리즘의 어긋난 그림자)은 옮긴 둥근 사각형 하나로.
 */
class SDrSurfaceDecor : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrSurfaceDecor) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { SetVisibility(EVisibility::HitTestInvisible); }
  float Radius = 0.f;
  bool bPill = false;

  void SetLayers(const TArray<FDrUIShadow>& InLayers) {
    Layers = InLayers;
    // 흐림 없는 겹은 둥근 상자 브러시로 그린다. 그리는 동안 살아 있어야 하므로 여기 둔다.
    HardBrushes.Reset();
    for (const FDrUIShadow& Layer : Layers) {
      FSlateRoundedBoxBrush Brush(FLinearColor::White, Radius + FMath::Max(Layer.Spread, 0.f));
      if (bPill) Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
      HardBrushes.Add(Brush);
    }
    Invalidate(EInvalidateWidgetReason::Paint);
  }

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    for (int32 I = 0; I < Layers.Num(); ++I) {
      const FDrUIShadow& Shadow = Layers[I];
      if (Shadow.Color.A <= 0.f) continue;
      if (Shadow.Blur <= 1.f) {
        const FVector2f Size = Geometry.GetLocalSize() + FVector2f(2.f * Shadow.Spread);
        const FGeometry Moved = Geometry.MakeChild(FVector2D(Size), FSlateLayoutTransform(FVector2f(Shadow.Offset) - FVector2f(Shadow.Spread)));
        FSlateDrawElement::MakeBox(Out, LayerId, Moved.ToPaintGeometry(), &HardBrushes[I], ESlateDrawEffect::None, Shadow.Color * Tint);
      } else PaintSoft(Shadow, Geometry, Out, LayerId, Tint);
    }
    return LayerId;
  }
private:
  TArray<FDrUIShadow> Layers;
  TArray<FSlateRoundedBoxBrush> HardBrushes;

  void PaintSoft(const FDrUIShadow& Shadow, const FGeometry& Geometry, FSlateWindowElementList& Out, int32 LayerId, const FLinearColor& Tint) const {
    const FVector2f Size = Geometry.GetLocalSize();
    const float R = bPill ? Size.Y * .5f : FMath::Min(Radius, FMath::Min(Size.X, Size.Y) * .5f);
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
  }
};

/**
 * 채움·테두리 위, 내용 아래에 윗면 반사광(윗변 안쪽 1px 선)을 그리는 SBorder.
 * SBorder는 브러시를 LayerId에, 내용을 LayerId + 1부터 그린다. 반사광을 LayerId에 더하면 내용이 표면 밖으로
 * 커져 윗변을 넘어도(독 확대 등) 선이 내용 위로 올라오지 않는다.
 */
class SDrSurfaceBorder : public SBorder {
public:
  void Construct(const SBorder::FArguments& Args) { SBorder::Construct(Args); }
  FLinearColor HighlightColor = FLinearColor::Transparent;
  float Radius = 0.f;
  bool bPill = false;
  /** 테마 이미지로 칠할 때의 9분할 브러시(그때 SBorder 자체 브러시는 비운다). */
  TOptional<FSlateBrush> ImageBrush;

  virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& Culling, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool bParentEnabled) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    if (ImageBrush) {
      // 9분할 여백이 표면보다 크면(얇은 머리줄·작은 칩) 모서리가 겹쳐 찌그러진다. 여백이 들어갈 만큼 그림 전체를 줄여 그린다.
      PaintBrush = *ImageBrush;
      const FVector2f Image(PaintBrush.ImageSize);
      const float MarginX = Image.X * (PaintBrush.Margin.Left + PaintBrush.Margin.Right), MarginY = Image.Y * (PaintBrush.Margin.Top + PaintBrush.Margin.Bottom);
      const float Fit = FMath::Min3(1.f, MarginX > 0.f ? Size.X / MarginX : 1.f, MarginY > 0.f ? Size.Y / MarginY : 1.f);
      PaintBrush.ImageSize = FVector2D(Image * Fit);
      FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(), &PaintBrush, ESlateDrawEffect::None, PaintBrush.GetTint(Style) * Style.GetColorAndOpacityTint());
    }
    const int32 MaxLayer = SBorder::OnPaint(Args, Geometry, Culling, Out, LayerId, Style, bParentEnabled);
    const float R = bPill ? Size.Y * .5f : FMath::Min(Radius, FMath::Min(Size.X, Size.Y) * .5f);
    if (HighlightColor.A <= 0.f || Size.X <= 2.f * R) return MaxLayer;
    // 윗변의 곧은 부분에만 1px 선(모서리 곡선 앞에서 멈춤).
    const float Inset = FMath::Max(R * .7f, 2.f);
    const TArray<FVector2f> Line = {FVector2f(Inset, .75f), FVector2f(Size.X - Inset, .75f)};
    FSlateDrawElement::MakeLines(Out, LayerId, Geometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, HighlightColor * Style.GetColorAndOpacityTint(), true, 1.f);
    return MaxLayer;
  }
private:
  /** 그리는 동안 살아 있어야 하는 줄인 브러시. */
  mutable FSlateBrush PaintBrush;
};

UDrSurface::UDrSurface() { SetHorizontalAlignment(HAlign_Fill); SetVerticalAlignment(VAlign_Fill); }

static EDrUISurfaceQuality CurrentQuality() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem ? Subsystem->GetSurfaceQuality() : GetDefault<UDrUIKitSettings>()->SurfaceQuality;
}

FName UDrSurface::ResolveFillToken() const {
  const bool bNoBlur = BackdropToken.IsNone() || CurrentQuality() == EDrUISurfaceQuality::Low;
  return bNoBlur && !FillStrongToken.IsNone() ? FillStrongToken : FillToken;
}

bool UDrSurface::HasFillImage() const {
  const FName Token = ResolveFillToken();
  return !Token.IsNone() && UDrUIThemeSubsystem::Current().Images.Contains(Token);
}

FLinearColor UDrSurface::ResolveFill() const {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const FName Token = ResolveFillToken();
  if (Token.IsNone()) return FLinearColor::Transparent;
  FLinearColor Color = Theme.GetColor(Token);
  if (Theme.TranslucentTokens.Contains(Token)) {
    const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
    Color.A = UDrUITheme::AdjustAlpha(Color.A, Subsystem ? Subsystem->GetSurfaceTransparency() : GetDefault<UDrUIKitSettings>()->SurfaceTransparency);
  }
  return Color;
}

float UDrSurface::ResolveBlur() const {
  // 그림으로 칠하는 표면은 흐리지 않는다(찢긴 종이처럼 가장자리가 사각형이 아니면 흐림이 삐져나온다).
  if (BackdropToken.IsNone() || HasFillImage()) return 0.f;
  const EDrUISurfaceQuality Quality = CurrentQuality();
  if (Quality == EDrUISurfaceQuality::Low) return 0.f;
  const float Strength = UDrUIThemeSubsystem::Current().GetBackdrop(BackdropToken);
  return Quality == EDrUISurfaceQuality::Medium ? Strength * .5f : Strength;
}

float UDrSurface::ResolveStrokeWidth() const {
  if (StrokeToken.IsNone()) return 0.f;
  // 테마가 기본 테두리 두께를 정하면(브루탈리즘) 1px 대신 그 두께, 부품이 더한 강조(포커스 2px 등)는 그 위에 얹는다.
  const float ThemeWidth = UDrUIThemeSubsystem::Current().StrokeWidth;
  return ThemeWidth > 0.f ? ThemeWidth + FMath::Max(StrokeWidth - 1.f, 0.f) : StrokeWidth;
}

void UDrSurface::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const float Radius = RadiusToken.IsNone() ? 0.f : Theme.GetRadius(RadiusToken);
  const FLinearColor Stroke = StrokeToken.IsNone() ? FLinearColor::Transparent : Theme.GetColor(StrokeToken);
  FSlateRoundedBoxBrush Brush(ResolveFill(), Radius, Stroke, ResolveStrokeWidth());
  // 알약(radius-pill = 999)은 반경을 그대로 쓰면 마름모처럼 그려진다. 높이의 절반으로 둥글린다.
  const bool bPill = Radius >= 500.f;
  if (bPill) Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  // 채움 토큰에 이미지가 있으면 9분할 그림(테두리도 그림이 맡는다).
  FSlateBrush ImageBrush;
  const bool bImage = Theme.MakeImageBrush(ResolveFillToken(), ImageBrush);
  if (bImage) SetBrush(FSlateNoResource());  // 그림은 SDrSurfaceBorder가 크기에 맞춰 그린다
  else SetBrush(Brush);
  if (!PaddingToken.IsNone()) SetPadding(FMargin(Theme.GetSpacing(PaddingToken)));
  if (Shadow) {
    Shadow->Radius = Radius;
    Shadow->bPill = bPill;
    Shadow->SetLayers(ShadowToken.IsNone() ? TArray<FDrUIShadow>() : Theme.GetShadowLayers(ShadowToken));
  }
  if (SurfaceBorder) {
    SurfaceBorder->ImageBrush = bImage ? TOptional<FSlateBrush>(ImageBrush) : TOptional<FSlateBrush>();
    SurfaceBorder->HighlightColor = HighlightToken.IsNone() || bImage ? FLinearColor::Transparent : Theme.GetColor(HighlightToken);
    SurfaceBorder->Radius = Radius;
    SurfaceBorder->bPill = bPill;
    SurfaceBorder->Invalidate(EInvalidateWidgetReason::Paint);
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
  // UBorder::RebuildWidget과 같되, 반사광을 내용 아래에 그리는 SDrSurfaceBorder를 쓴다.
  MyBorder = SAssignNew(SurfaceBorder, SDrSurfaceBorder).FlipForRightToLeftFlowDirection(bFlipForRightToLeftFlowDirection);
  if (GetChildrenCount() > 0) Cast<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
  // 흐림 강도 0이면 SBackgroundBlur는 그리지 않는다. 품질이 바뀌어도 트리를 다시 만들 필요가 없게 늘 감싼다.
  SAssignNew(Blur, SBackgroundBlur).BlurStrength(0.f).bApplyAlphaToBlur(true).Padding(0.f)[MyBorder.ToSharedRef()];
  // 뒤 그림자 · 흐림+표면(채움·테두리 → 반사광 → 내용). 그림자가 없으면 그리지 않는다.
  SAssignNew(Shadow, SDrSurfaceDecor);
  ApplyTheme();
  return SNew(SOverlay) + SOverlay::Slot()[Shadow.ToSharedRef()] + SOverlay::Slot()[Blur.ToSharedRef()];
}

void UDrSurface::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Blur.Reset();
  Shadow.Reset();
  SurfaceBorder.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
