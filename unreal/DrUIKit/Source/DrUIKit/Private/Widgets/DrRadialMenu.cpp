#include "Widgets/DrRadialMenu.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/SizeBox.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrStatus.h"

namespace {
bool IsRadialReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
}

UDrRadialMenu::UDrRadialMenu(const FObjectInitializer& Initializer) : Super(Initializer) {
  SetIsFocusable(true);                       // 패드·키보드 입력을 받는다
  SetVisibility(ESlateVisibility::Visible);   // 마우스 방향을 받으려면 눌림 판정이 있어야 한다
  Sweep.Snap(0.f);
  Shown.Snap(0.f);
}

int32 UDrRadialMenu::SectorAt(FVector2D Direction, int32 Count) {
  if (Count <= 0 || Direction.IsNearlyZero()) return INDEX_NONE;
  // 12시가 0, 시계 방향(화면 y는 아래로 커지므로 위쪽이 -y).
  float Angle = FMath::Atan2(float(Direction.X), float(-Direction.Y));
  if (Angle < 0.f) Angle += 2.f * PI;
  return FMath::FloorToInt(Angle / (2.f * PI) * Count + .5f) % Count;
}

float UDrRadialMenu::NearestWrapped(float From, int32 To, int32 Count) {
  if (Count <= 0) return float(To);
  float Delta = float(To) - From;
  Delta -= Count * FMath::RoundToFloat(Delta / Count);
  return From + Delta;
}

void UDrRadialMenu::SetItems(const TArray<FText>& InItems) {
  Items = InItems;
  if (!Items.IsValidIndex(Highlighted)) SetHighlighted(INDEX_NONE);
}

void UDrRadialMenu::SetHighlighted(int32 Index) {
  const int32 Next = Items.IsValidIndex(Index) ? Index : INDEX_NONE;
  if (Next == Highlighted) return;
  const bool bAnimate = bTicked && !IsRadialReduceMotion();
  if (Next != INDEX_NONE) {
    if (Highlighted == INDEX_NONE || !bAnimate) Sweep.Snap(float(Next));  // 처음 가리키면 그 자리에서 나타난다
    else Sweep.Target = NearestWrapped(Sweep.Value, Next, Items.Num());  // 가까운 쪽으로 돈다
  }
  Highlighted = Next;
  if (bAnimate) Shown.Target = Next != INDEX_NONE ? 1.f : 0.f;
  else Shown.Snap(Next != INDEX_NONE ? 1.f : 0.f);
}

void UDrRadialMenu::Confirm() {
  if (Items.IsValidIndex(Highlighted)) OnItemSelected.Broadcast(Highlighted);
}

void UDrRadialMenu::PointAt(FVector2D Direction) { SetHighlighted(SectorAt(Direction, Items.Num())); }

TSharedRef<SWidget> UDrRadialMenu::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    // 그림은 NativePaint가 그린다. 트리는 크기만 정한다(고리 + 포커스 링 여백).
    USizeBox* Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetWidthOverride(2.f * Radius + 12.f);
    Frame->SetHeightOverride(2.f * Radius + 12.f);
    WidgetTree->RootWidget = Frame;
  }
  return Super::RebuildWidget();
}

void UDrRadialMenu::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  bTicked = true;
  if (IsRadialReduceMotion()) { Sweep.Snap(Sweep.Target); Shown.Snap(Shown.Target); return; }
  if (!Sweep.IsSettled()) Sweep.Step(DeltaTime);
  if (!Shown.IsSettled()) Shown.Step(DeltaTime);
}

int32 UDrRadialMenu::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& Out, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
  int32 Layer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, Out, LayerId, InWidgetStyle, bParentEnabled) + 1;
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  const FLinearColor Tint = InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.f, 1.f, 1.f, bParentEnabled && GetIsEnabled() ? 1.f : .4f);
  const FVector2f Size = AllottedGeometry.GetLocalSize();
  const FVector2f Center = Size * .5f;
  const float Outer = FMath::Min(Radius, FMath::Min(Size.X, Size.Y) * .5f - 6.f);
  const float Inner = FMath::Clamp(InnerRadius, 8.f, Outer - 8.f);
  const float Mid = (Outer + Inner) * .5f, Thick = Outer - Inner;
  const int32 Count = Items.Num();
  // 굵은 선은 이음매가 겹치므로 반투명이면 얼룩진다. 칸 색은 불투명으로 섞어 만든다.
  // 유리 카드 위에 올라가므로 칸은 바탕(glass-strong)보다 살짝 진하게, 가운데 원은 바탕색 그대로(고리와 구분).
  const FLinearColor Surface = Theme.GetColor(DrUITokens::GlassStrong).CopyWithNewOpacity(1.f);
  const FLinearColor Base = FMath::Lerp(Surface, Theme.GetColor(DrUITokens::Ink), .08f).CopyWithNewOpacity(1.f);
  const FLinearColor Accent = Theme.GetColor(DrUITokens::Accent);
  const FLinearColor Lit = FMath::Lerp(Surface, Accent, .25f).CopyWithNewOpacity(1.f);
  const float Gap = Count > 0 ? FMath::Min(.006f, .15f / Count) : 0.f;
  auto Arc = [&](float ArcRadius, float Start, float Length, float Width, const FLinearColor& Color) {
    const TArray<FVector2f> Points = DrProgressRing::ArcPoints(Center, ArcRadius, Start, Length, 160);
    if (Points.Num() > 1) FSlateDrawElement::MakeLines(Out, Layer, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color * Tint, true, Width);
  };
  for (int32 I = 0; I < Count; ++I) Arc(Mid, (I - .5f) / Count + Gap, 1.f / Count - 2.f * Gap, Thick, Base);
  ++Layer;
  if (Count > 0 && Shown.Value > .01f) {
    const float Start = (Sweep.Value - .5f) / Count + Gap, Length = 1.f / Count - 2.f * Gap;
    const float Alpha = FMath::Clamp(Shown.Value, 0.f, 1.f);
    Arc(Mid, Start, Length, Thick, Lit.CopyWithNewOpacity(Alpha));
    ++Layer;
    Arc(Outer - 1.5f, Start, Length, 3.f, Accent.CopyWithNewOpacity(Alpha));  // 바깥 테두리 강조
    ++Layer;
  }
  // 가운데 원 + 글.
  FSlateRoundedBoxBrush Disc(FLinearColor::White, 999.f);
  Disc.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
  const float DiscSize = 2.f * (Inner - 6.f);
  FSlateDrawElement::MakeBox(Out, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(DiscSize), FSlateLayoutTransform(Center - FVector2f(DiscSize * .5f))), &Disc, ESlateDrawEffect::None, Surface * Tint);
  ++Layer;
  const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
  auto Text = [&](const FText& InText, FName Style, FName Color, FVector2f At) {
    const FSlateFontInfo Font = Theme.MakeFont(Style);
    const FVector2D TextSize = Measure->Measure(InText, Font);
    FSlateDrawElement::MakeText(Out, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(TextSize), FSlateLayoutTransform(At - FVector2f(TextSize) * .5f)), InText, Font, ESlateDrawEffect::None, Theme.GetColor(Color) * Tint);
  };
  for (int32 I = 0; I < Count; ++I) {
    const float Angle = 2.f * PI * I / Count;
    Text(Items[I], DrUITokens::Label, I == Highlighted ? DrUITokens::Ink : DrUITokens::InkMuted, Center + Mid * FVector2f(FMath::Sin(Angle), -FMath::Cos(Angle)));
  }
  if (Items.IsValidIndex(Highlighted)) Text(Items[Highlighted], TEXT("heading"), DrUITokens::Ink, Center);
  else Text(Hint, DrUITokens::Caption, DrUITokens::InkMuted, Center);
  ++Layer;
  if (HasAnyUserFocus()) Arc(Outer + 4.f, 0.f, 1.f, 2.f, Theme.GetColor(DrUITokens::Focus));  // 키보드·패드 포커스 링
  return Layer;
}

FReply UDrRadialMenu::NativeOnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) {
  const FVector2D Local = FVector2D(Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())) - FVector2D(Geometry.GetLocalSize()) * .5;
  // 가운데 원 안은 '아직 안 고름'(방향이 너무 짧아 흔들린다).
  if (Local.Size() < InnerRadius * .5f) SetHighlighted(INDEX_NONE);
  else PointAt(Local);
  return Super::NativeOnMouseMove(Geometry, Event);
}

FReply UDrRadialMenu::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (Event.GetEffectingButton() == EKeys::LeftMouseButton) {
    FReply Reply = FReply::Handled();
    if (TSharedPtr<SWidget> Slate = GetCachedWidget()) Reply.SetUserFocus(Slate.ToSharedRef(), EFocusCause::Mouse);  // 이후 키보드·패드도 여기로
    return Reply;
  }
  return Super::NativeOnMouseButtonDown(Geometry, Event);
}

FReply UDrRadialMenu::NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& Event) {
  if (Event.GetEffectingButton() == EKeys::LeftMouseButton) { Confirm(); return FReply::Handled(); }
  return Super::NativeOnMouseButtonUp(Geometry, Event);
}

void UDrRadialMenu::NativeOnMouseLeave(const FPointerEvent& Event) {
  Super::NativeOnMouseLeave(Event);  // 강조는 남긴다(밖으로 살짝 나가도 고른 칸 유지)
}

FReply UDrRadialMenu::NativeOnAnalogValueChanged(const FGeometry& Geometry, const FAnalogInputEvent& Event) {
  const FKey Key = Event.GetKey();
  const float Value = Event.GetAnalogValue();
  if (Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_RightX) Stick.X = Value;
  else if (Key == EKeys::Gamepad_LeftY || Key == EKeys::Gamepad_RightY) Stick.Y = -Value;  // 스틱 위가 +, 화면 위는 -y
  else return Super::NativeOnAnalogValueChanged(Geometry, Event);
  if (Stick.Size() > StickDeadZone) PointAt(Stick);  // 놓으면(가운데) 마지막 칸 유지
  return FReply::Handled();
}

FReply UDrRadialMenu::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) {
  const FKey Key = Event.GetKey();
  if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom) { Confirm(); return FReply::Handled(); }
  if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right) { OnCancelled.Broadcast(); return FReply::Handled(); }
  return Super::NativeOnKeyDown(Geometry, Event);
}

FNavigationReply UDrRadialMenu::NativeOnNavigation(const FGeometry& Geometry, const FNavigationEvent& Event, const FNavigationReply& DefaultReply) {
  // 스틱은 아날로그 값으로 따로 받으므로 패드에서 온 탐색은 먹기만 한다(두 번 움직이지 않게).
  if (Event.GetNavigationGenesis() == ENavigationGenesis::Controller) return FNavigationReply::Stop();
  const int32 Count = Items.Num();
  switch (Event.GetNavigationType()) {
  case EUINavigation::Right: case EUINavigation::Down:
    if (Count > 0) SetHighlighted(Highlighted == INDEX_NONE ? 0 : (Highlighted + 1) % Count);
    return FNavigationReply::Stop();
  case EUINavigation::Left: case EUINavigation::Up:
    if (Count > 0) SetHighlighted(Highlighted == INDEX_NONE ? 0 : (Highlighted + Count - 1) % Count);
    return FNavigationReply::Stop();
  default:
    return Super::NativeOnNavigation(Geometry, Event, DefaultReply);
  }
}
