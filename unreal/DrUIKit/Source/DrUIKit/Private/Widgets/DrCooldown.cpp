#include "Widgets/DrCooldown.h"
#include "Core/DrUIDraw.h"
#include "Fonts/FontMeasure.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/SLeafWidget.h"

namespace {
double Now() { return FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetCurrentTime() : FPlatformTime::Seconds(); }
}

FText UDrCooldown::FormatRemaining(float Remaining) {
  if (Remaining <= 0.f) return FText::GetEmpty();
  if (Remaining >= 10.f) return FText::AsNumber(FMath::CeilToInt(Remaining));
  FNumberFormattingOptions Format;
  Format.SetMinimumFractionalDigits(1).SetMaximumFractionalDigits(1).SetRoundingMode(ERoundingMode::ToPositiveInfinity);
  return FText::AsNumber(Remaining, &Format);
}

TArray<FVector2f> UDrCooldown::SweepPolygon(FVector2f Size, float InFraction) {
  TArray<FVector2f> Points;
  const float F = FMath::Clamp(InFraction, 0.f, 1.f);
  if (F <= 0.f) return Points;
  const FVector2f Center = Size * .5f;
  Points.Add(Center);
  // 남은 부분 = 지금 각도(1 - F 바퀴)부터 한 바퀴 끝(12시)까지. 각도마다 중심에서 쏜 선이 사각형 변과 만나는 점.
  const int32 Steps = FMath::Max(2, FMath::CeilToInt(F * 64.f));
  for (int32 I = 0; I <= Steps; ++I) {
    const float Turn = (1.f - F) + F * I / Steps;
    const float Angle = 2.f * PI * Turn - .5f * PI;
    const FVector2f Dir(FMath::Cos(Angle), FMath::Sin(Angle));
    const float Tx = FMath::Abs(Dir.X) > 1e-5f ? Center.X / FMath::Abs(Dir.X) : TNumericLimits<float>::Max();
    const float Ty = FMath::Abs(Dir.Y) > 1e-5f ? Center.Y / FMath::Abs(Dir.Y) : TNumericLimits<float>::Max();
    Points.Add(Center + Dir * FMath::Min(Tx, Ty));
  }
  return Points;
}

class SDrCooldown : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrCooldown) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) { SetVisibility(EVisibility::HitTestInvisible); }
  double EndTime = 0.0, FlashStart = -100.0;
  float Total = 0.f, Radius = 8.f;
  bool bShowSeconds = true, bReduceMotion = false;
  FLinearColor Scrim = FLinearColor(0, 0, 0, .5f), Ink = FLinearColor::White, Flash = FLinearColor::Blue;
  FSlateFontInfo Font;
  TFunction<void()> OnFinished;

  void Restart() {
    if (!Timer.IsValid()) Timer = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrCooldown::Animate));
    Invalidate(EInvalidateWidgetReason::Paint);
  }
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }  // 아이콘 크기를 따른다
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    const FVector2f Size = Geometry.GetLocalSize();
    const FLinearColor Tint = Style.GetColorAndOpacityTint();
    const double T = Now();
    const float Remaining = float(FMath::Max(0.0, EndTime - T));
    const float F = UDrCooldown::Fraction(Remaining, Total);
    if (F > 0.f) {
      // 부채꼴(가운데에서 부채살) 삼각형 팬.
      const TArray<FVector2f> Poly = UDrCooldown::SweepPolygon(Size, F);
      TArray<FLinearColor> Colors;
      Colors.Init(Scrim * Tint, Poly.Num());
      TArray<SlateIndex> Indices;
      for (int32 I = 1; I + 1 < Poly.Num(); ++I) Indices.Append({0, SlateIndex(I), SlateIndex(I + 1)});
      DrUIDraw::Triangles(Out, LayerId, Geometry, Poly, Colors, Indices);
      if (bShowSeconds) {
        const FText Text = UDrCooldown::FormatRemaining(Remaining);
        const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text, Font);
        FSlateDrawElement::MakeText(Out, LayerId + 1, Geometry.ToPaintGeometry(FVector2f(TextSize), FSlateLayoutTransform((Size - FVector2f(TextSize)) * .5f)), Text, Font, ESlateDrawEffect::None, Ink * Tint);
      }
    }
    // 끝나면 0.45초 동안 테두리가 반짝였다 사라진다.
    const float FlashAge = float(T - FlashStart);
    if (!bReduceMotion && FlashAge >= 0.f && FlashAge < .45f) {
      const TArray<FVector2f> Ring = DrUIDraw::RoundedRectPath(FVector2f(1.f), Size - FVector2f(2.f), Radius - 1.f);
      FSlateDrawElement::MakeLines(Out, LayerId + 2, Geometry.ToPaintGeometry(), Ring, ESlateDrawEffect::None, Flash.CopyWithNewOpacity(Flash.A * (1.f - FlashAge / .45f)) * Tint, true, 2.f);
    }
    return LayerId + 2;
  }
private:
  EActiveTimerReturnType Animate(double, float) {
    Invalidate(EInvalidateWidgetReason::Paint);
    const double T = Now();
    if (EndTime > 0.0 && T >= EndTime) {
      EndTime = 0.0;
      FlashStart = T;
      if (OnFinished) OnFinished();
    }
    if (EndTime > 0.0 || T - FlashStart < .45) return EActiveTimerReturnType::Continue;
    Timer.Reset();
    return EActiveTimerReturnType::Stop;
  }
  TSharedPtr<FActiveTimerHandle> Timer;
};

void UDrCooldown::StartCooldown(float Seconds) { SetRemaining(Seconds, Seconds); }

void UDrCooldown::SetRemaining(float Remaining, float InTotal) {
  Total = FMath::Max(InTotal, Remaining);
  EndTime = Remaining > 0.f ? Now() + Remaining : 0.0;
  if (Slate) { Slate->EndTime = EndTime; Slate->Total = Total; Slate->Restart(); }
}

bool UDrCooldown::IsCoolingDown() const { return EndTime > Now(); }
float UDrCooldown::GetRemaining() const { return float(FMath::Max(0.0, EndTime - Now())); }

void UDrCooldown::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  Slate->Scrim = Theme.GetColor(ScrimToken).CopyWithNewOpacity(FMath::Max(Theme.GetColor(ScrimToken).A, .5f));  // 아이콘 위에서 읽히게 최소 50%
  Slate->Ink = Theme.GetColor(TextToken);
  Slate->Flash = Theme.GetColor(FlashToken);
  Slate->Radius = Theme.GetRadius(RadiusToken);
  Slate->Font = Theme.MakeFont(TEXT("label"));
  Slate->bShowSeconds = bShowSeconds;
  Slate->bReduceMotion = Subsystem && Subsystem->IsReduceMotion();
  Slate->Invalidate(EInvalidateWidgetReason::Paint);
}

TSharedRef<SWidget> UDrCooldown::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrCooldown);
  Slate->OnFinished = [WeakThis = TWeakObjectPtr<UDrCooldown>(this)] { if (UDrCooldown* This = WeakThis.Get()) { This->ApplyTheme(); This->OnFinished.Broadcast(); } };
  Slate->EndTime = EndTime;
  Slate->Total = Total;
  ApplyTheme();
  if (EndTime > 0.0) Slate->Restart();
  return Slate.ToSharedRef();
}

void UDrCooldown::SynchronizeProperties() { Super::SynchronizeProperties(); ApplyTheme(); }

void UDrCooldown::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
