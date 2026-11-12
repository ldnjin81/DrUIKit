#include "Widgets/DrUISurface.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/Layout/SBackgroundBlur.h"

UDrUISurface::UDrUISurface() { SetHorizontalAlignment(HAlign_Fill); SetVerticalAlignment(VAlign_Fill); }

static EDrUISurfaceQuality CurrentQuality() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem ? Subsystem->GetSurfaceQuality() : GetDefault<UDrUIKitSettings>()->SurfaceQuality;
}

FLinearColor UDrUISurface::ResolveFill() const {
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

float UDrUISurface::ResolveBlur() const {
  if (BackdropToken.IsNone()) return 0.f;
  const EDrUISurfaceQuality Quality = CurrentQuality();
  if (Quality == EDrUISurfaceQuality::Low) return 0.f;
  const float Strength = UDrUIThemeSubsystem::Current().GetBackdrop(BackdropToken);
  return Quality == EDrUISurfaceQuality::Medium ? Strength * .5f : Strength;
}

void UDrUISurface::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const float Radius = RadiusToken.IsNone() ? 0.f : Theme.GetRadius(RadiusToken);
  const FLinearColor Stroke = StrokeToken.IsNone() ? FLinearColor::Transparent : Theme.GetColor(StrokeToken);
  FSlateRoundedBoxBrush Brush(ResolveFill(), Radius, Stroke, StrokeToken.IsNone() ? 0.f : StrokeWidth);
  // 알약(radius-pill = 999)은 반경을 그대로 쓰면 마름모처럼 그려진다. 높이의 절반으로 둥글린다.
  const bool bPill = Radius >= 500.f;
  if (bPill) Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  SetBrush(Brush);
  if (!PaddingToken.IsNone()) SetPadding(FMargin(Theme.GetSpacing(PaddingToken)));
  if (Blur) {
    Blur->SetBlurStrength(ResolveBlur());
    // 흐림은 반경을 그대로 받으므로 알약은 적당히 크게(대부분의 바 높이의 절반 이상)만 준다.
    const float BlurRadius = bPill ? 24.f : Radius;
    Blur->SetCornerRadius(FVector4(BlurRadius, BlurRadius, BlurRadius, BlurRadius));
  }
}

void UDrUISurface::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrUISurface::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  TSharedRef<SWidget> Inner = Super::RebuildWidget();
  // 흐림 강도 0이면 SBackgroundBlur는 그리지 않는다. 품질이 바뀌어도 트리를 다시 만들 필요가 없게 늘 감싼다.
  SAssignNew(Blur, SBackgroundBlur).BlurStrength(0.f).bApplyAlphaToBlur(true).Padding(0.f)[Inner];
  return Blur.ToSharedRef();
}

void UDrUISurface::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Blur.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
