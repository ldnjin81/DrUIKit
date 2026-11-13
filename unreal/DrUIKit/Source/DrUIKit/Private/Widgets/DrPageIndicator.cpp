#include "Widgets/DrPageIndicator.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Core/DrUIMotion.h"
#include "Rendering/DrawElements.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/SLeafWidget.h"

float DrPageIndicatorLayout::DotWidth(int32 Index, float Active, float Dot, float Wide) {
  // 지금 페이지와의 거리 0이면 Wide, 1 이상이면 Dot. 사이는 선형(스프링이 옮겨 가는 동안 두 점이 함께 변한다).
  const float Near = FMath::Clamp(1.f - FMath::Abs(Index - Active), 0.f, 1.f);
  return FMath::Lerp(Dot, Wide, Near);
}

float DrPageIndicatorLayout::TotalWidth(int32 Count, float Dot, float Wide, float Gap) {
  return Count <= 0 ? 0.f : (Count - 1) * (Dot + Gap) + Wide;
}

class SDrPageIndicator : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrPageIndicator) {}
  SLATE_END_ARGS()
  void Construct(const FArguments&) {
    Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
    Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
    Active.Snap(0.f);
  }
  int32 Count = 3;
  float Dot = 8.f, Wide = 22.f, Gap = 8.f;
  FLinearColor DotColor = FLinearColor::Gray, ActiveColor = FLinearColor::White;
  TFunction<void(int32)> OnClicked;

  void SetTarget(int32 Page, bool bAnimate) {
    if (bAnimate) {
      Active.Target = float(Page);
      if (!TimerHandle.IsValid()) TimerHandle = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SDrPageIndicator::Animate));
    } else {
      Active.Snap(float(Page));
    }
    Invalidate(EInvalidateWidgetReason::Paint);
  }

  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(DrPageIndicatorLayout::TotalWidth(Count, Dot, Wide, Gap), FMath::Max(Dot, 12.f)); }

  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool bParentEnabled) const override {
    const FLinearColor Tint = Style.GetColorAndOpacityTint() * FLinearColor(1.f, 1.f, 1.f, bParentEnabled ? 1.f : .4f);
    const float Y = (Geometry.GetLocalSize().Y - Dot) * .5f;
    float X = 0.f;
    for (int32 I = 0; I < Count; ++I) {
      const float Width = DrPageIndicatorLayout::DotWidth(I, Active.Value, Dot, Wide);
      const float Near = (Width - Dot) / FMath::Max(Wide - Dot, 1.f);
      FSlateDrawElement::MakeBox(Out, LayerId, Geometry.ToPaintGeometry(FVector2f(Width, Dot), FSlateLayoutTransform(FVector2f(X, Y))), &Brush, ESlateDrawEffect::None,
        FMath::Lerp(DotColor, ActiveColor, Near) * Tint);
      X += Width + Gap;
    }
    return LayerId;
  }

  virtual FReply OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override {
    if (Event.GetEffectingButton() != EKeys::LeftMouseButton || !OnClicked) return FReply::Unhandled();
    // 누른 x에 가장 가까운 점(점 사이 틈은 반씩 나눔).
    const float LocalX = Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition()).X;
    float X = 0.f;
    for (int32 I = 0; I < Count; ++I) {
      const float Width = DrPageIndicatorLayout::DotWidth(I, Active.Value, Dot, Wide);
      if (LocalX < X + Width + Gap * .5f || I == Count - 1) { OnClicked(I); return FReply::Handled(); }
      X += Width + Gap;
    }
    return FReply::Unhandled();
  }
  virtual FCursorReply OnCursorQuery(const FGeometry&, const FPointerEvent&) const override { return FCursorReply::Cursor(EMouseCursor::Hand); }

private:
  EActiveTimerReturnType Animate(double, float DeltaTime) {
    const bool bSettled = Active.Step(DeltaTime);
    Invalidate(EInvalidateWidgetReason::Paint);
    if (bSettled) { TimerHandle.Reset(); return EActiveTimerReturnType::Stop; }
    return EActiveTimerReturnType::Continue;
  }
  FSlateRoundedBoxBrush Brush = FSlateRoundedBoxBrush(FLinearColor::White, 999.f);
  FDrUISpring Active;
  TSharedPtr<FActiveTimerHandle> TimerHandle;
};

void UDrPageIndicator::SetPageCount(int32 InCount) {
  PageCount = FMath::Max(0, InCount);
  Current = FMath::Clamp(Current, 0, FMath::Max(0, PageCount - 1));
  ApplyTheme();
}

void UDrPageIndicator::SetCurrent(int32 Page) {
  Current = FMath::Clamp(Page, 0, FMath::Max(0, PageCount - 1));
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Slate) Slate->SetTarget(Current, !(Subsystem && Subsystem->IsReduceMotion()));
}

void UDrPageIndicator::ApplyTheme() {
  if (!Slate) return;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  Slate->Count = PageCount;
  Slate->Dot = DotSize;
  Slate->Wide = FMath::Max(ActiveWidth, DotSize);
  Slate->Gap = Gap;
  Slate->DotColor = Theme.GetColor(DotToken);
  Slate->ActiveColor = Theme.GetColor(ActiveToken);
  Slate->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UDrPageIndicator::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  SAssignNew(Slate, SDrPageIndicator);
  Slate->OnClicked = [WeakThis = TWeakObjectPtr<UDrPageIndicator>(this)](int32 Page) {
    if (UDrPageIndicator* This = WeakThis.Get()) This->OnPageClicked.Broadcast(Page);
  };
  ApplyTheme();
  Slate->SetTarget(Current, false);  // 처음은 제자리
  return Slate.ToSharedRef();
}

void UDrPageIndicator::SynchronizeProperties() {
  Super::SynchronizeProperties();
  ApplyTheme();
}

void UDrPageIndicator::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Slate.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
