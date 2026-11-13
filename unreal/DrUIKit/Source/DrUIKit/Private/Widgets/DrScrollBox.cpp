#include "Widgets/DrScrollBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Core/DrUIDraw.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"

/** 스크롤 가장자리 그늘(더 볼 내용이 있는 쪽에만). */
class SDrScrollEdge : public SLeafWidget {
public:
  SLATE_BEGIN_ARGS(SDrScrollEdge) {}
  SLATE_ATTRIBUTE(bool, Active)
  SLATE_END_ARGS()
  void Construct(const FArguments& Args) { Active = Args._Active; SetVisibility(EVisibility::HitTestInvisible); }
  bool bStart = true, bVertical = true;
  float Size = 14.f;
  FLinearColor Color = FLinearColor::Black;
  virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
  virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& Style, bool) const override {
    if (!Active.Get()) return LayerId;
    const FVector2f Local = Geometry.GetLocalSize();
    const FLinearColor Edge = Color * Style.GetColorAndOpacityTint(), Clear = Edge.CopyWithNewOpacity(0.f);
    if (bVertical) {
      const FVector2f Band(Local.X, FMath::Min(Size, Local.Y * .5f));
      if (bStart) DrUIDraw::Gradient(Out, LayerId, Geometry, FVector2f::ZeroVector, Band, Edge, Clear, true);
      else DrUIDraw::Gradient(Out, LayerId, Geometry, FVector2f(0.f, Local.Y - Band.Y), Band, Clear, Edge, true);
    } else {
      const FVector2f Band(FMath::Min(Size, Local.X * .5f), Local.Y);
      if (bStart) DrUIDraw::Gradient(Out, LayerId, Geometry, FVector2f::ZeroVector, Band, Edge, Clear, false);
      else DrUIDraw::Gradient(Out, LayerId, Geometry, FVector2f(Local.X - Band.X, 0.f), Band, Clear, Edge, false);
    }
    return LayerId;
  }
private:
  TAttribute<bool> Active;
};

void UDrScrollBox::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  // 브러시 기본 크기(32px)가 막대 굵기로 쓰이므로 굵기만큼으로 준다.
  auto Thumb = [&](FName Token) { return FSlateRoundedBoxBrush(Theme.GetColor(Token), BarThickness * .5f, FVector2D(BarThickness)); };
  FSlateBrush None;
  None.DrawAs = ESlateBrushDrawType::NoDrawType;
  FScrollBarStyle Bar = GetWidgetBarStyle();
  Bar.SetVerticalBackgroundImage(None).SetHorizontalBackgroundImage(None)
    .SetVerticalTopSlotImage(None).SetVerticalBottomSlotImage(None).SetHorizontalTopSlotImage(None).SetHorizontalBottomSlotImage(None)
    .SetNormalThumbImage(Thumb(ThumbToken)).SetHoveredThumbImage(Thumb(ThumbHoverToken)).SetDraggedThumbImage(Thumb(ThumbHoverToken))
    .SetThickness(BarThickness);
  SetWidgetBarStyle(Bar);
  // 엔진 기본 스크롤 그림자(검은 그라데이션 이미지)는 끈다. 가장자리 그늘은 SDrScrollEdge가 토큰 색으로 그린다.
  FScrollBoxStyle Box = GetWidgetStyle();
  Box.SetTopShadowBrush(None).SetBottomShadowBrush(None).SetLeftShadowBrush(None).SetRightShadowBrush(None);
  SetWidgetStyle(Box);
  SetScrollbarThickness(FVector2D(BarThickness));
  const FLinearColor Edge = Theme.GetColor(EdgeToken).CopyWithNewOpacity(.08f);
  const bool bVertical = GetOrientation() == Orient_Vertical;
  for (SDrScrollEdge* E : {StartEdge.Get(), EndEdge.Get()})
    if (E) { E->Color = Edge; E->Size = EdgeSize; E->bVertical = bVertical; E->Invalidate(EInvalidateWidgetReason::Paint); }
}

TSharedRef<SWidget> UDrScrollBox::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  TSharedRef<SWidget> Box = Super::RebuildWidget();
  // 더 볼 내용이 있을 때만(시작 쪽은 조금이라도 내려갔을 때, 끝 쪽은 끝에 닿기 전).
  SAssignNew(StartEdge, SDrScrollEdge).Active(TAttribute<bool>::CreateWeakLambda(this, [this] { return bEdgeShadows && GetScrollOffset() > 1.f; }));
  SAssignNew(EndEdge, SDrScrollEdge).Active(TAttribute<bool>::CreateWeakLambda(this, [this] { return bEdgeShadows && GetScrollOffset() < GetScrollOffsetOfEnd() - 1.f; }));
  EndEdge->bStart = false;
  ApplyTheme();
  return SNew(SOverlay) + SOverlay::Slot()[Box] + SOverlay::Slot()[StartEdge.ToSharedRef()] + SOverlay::Slot()[EndEdge.ToSharedRef()];
}

void UDrScrollBox::SynchronizeProperties() {
  Super::SynchronizeProperties();
  ApplyTheme();
}

void UDrScrollBox::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  StartEdge.Reset();
  EndEdge.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
