#include "Widgets/DrQuantityStepper.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

// ---------------------------------------------------------------- 아이콘 버튼

UDrIconButton::UDrIconButton(const FObjectInitializer& Initializer) : Super(Initializer) { PressedScale = .92f; }

void UDrIconButton::BuildDefaultTree() {
  USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Box"));
  Box->SetWidthOverride(ButtonSize);
  Box->SetHeightOverride(ButtonSize);
  WidgetTree->RootWidget = Box;
  Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
  Surface->FillToken = NAME_None;
  Surface->FillStrongToken = NAME_None;
  Surface->BackdropToken = NAME_None;
  Surface->RadiusToken = DrUITokens::RadiusPill;
  Surface->PaddingToken = NAME_None;
  Surface->SetHorizontalAlignment(HAlign_Center);
  Surface->SetVerticalAlignment(VAlign_Center);
  Box->AddChild(Surface);
  Glyph = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Glyph"));
  Glyph->GlyphSize = FMath::RoundToFloat(ButtonSize * .5f);
  Surface->SetContent(Glyph);
}

void UDrIconButton::ApplyVisualState() {
  SetRenderOpacity(VisualState == EDrUIState::Disabled ? .35f : 1.f);
  if (Surface) {
    const bool bLens = VisualState == EDrUIState::Hover || VisualState == EDrUIState::Pressed;
    Surface->FillToken = bLens ? DrUITokens::Hairline : NAME_None;
    Surface->StrokeToken = bFocusedNow && DrUIState::IsFocusRingVisible(GetStateInputs()) ? DrUITokens::Focus : NAME_None;
    Surface->StrokeWidth = 2.f;
    Surface->ApplyTheme();
  }
  if (Glyph) { Glyph->SetShape(Shape); Glyph->SetColorToken(DrUITokens::Ink); }
}

// ---------------------------------------------------------------- 수량 스테퍼

float UDrQuantityStepper::RepeatInterval(float Held) {
  if (Held < .4f) return 0.f;
  return FMath::Lerp(.12f, .04f, FMath::Clamp((Held - .4f) / 2.f, 0.f, 1.f));
}

TSharedRef<SWidget> UDrQuantityStepper::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
    Surface->FillToken = DrUITokens::Hairline;
    Surface->FillStrongToken = NAME_None;
    Surface->BackdropToken = NAME_None;
    Surface->StrokeToken = NAME_None;
    Surface->RadiusToken = DrUITokens::RadiusPill;
    Surface->PaddingToken = NAME_None;
    Surface->SetPadding(FMargin(4.f));
    WidgetTree->RootWidget = Surface;
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Surface->SetContent(Row);
    MinusButton = WidgetTree->ConstructWidget<UDrIconButton>(UDrIconButton::StaticClass(), TEXT("MinusButton"));
    MinusButton->Shape = EDrGlyphShape::Minus;
    Row->AddChildToHorizontalBox(MinusButton);
    USizeBox* ValueBox = WidgetTree->ConstructWidget<USizeBox>();
    ValueBox->SetMinDesiredWidth(44.f);
    ValueBox->SetClipping(EWidgetClipping::ClipToBounds);
    ValueText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("ValueText"));
    ValueText->SetJustification(ETextJustify::Center);
    ValueBox->AddChild(ValueText);
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(ValueBox)) S->SetVerticalAlignment(VAlign_Center);
    PlusButton = WidgetTree->ConstructWidget<UDrIconButton>(UDrIconButton::StaticClass(), TEXT("PlusButton"));
    PlusButton->Shape = EDrGlyphShape::Plus;
    Row->AddChildToHorizontalBox(PlusButton);
    BindButtons();
    Slide.Snap(0.f);
    ApplyVisualState();
  }
  return Super::RebuildWidget();
}

void UDrQuantityStepper::NativeOnInitialized() {
  Super::NativeOnInitialized();
  BindButtons();  // 위젯 블루프린트가 둔 버튼
}

void UDrQuantityStepper::BindButtons() {
  if (bBound || !MinusButton || !PlusButton) return;
  bBound = true;
  // 클릭이 아니라 누름에 반응한다(누르고 있으면 반복). 클릭까지 받으면 한 번에 두 칸 움직인다.
  for (const auto& [Button, Direction] : TArray<TPair<UDrIconButton*, int32>>{{MinusButton, -1}, {PlusButton, 1}}) {
    const int32 Dir = Direction;
    Button->OnPressed().AddWeakLambda(this, [this, Dir] { Nudge(Dir); HeldDirection = Dir; HeldSeconds = 0.f; RepeatWait = 0.f; });
    Button->OnReleased().AddWeakLambda(this, [this] { HeldDirection = 0; });
  }
}

void UDrQuantityStepper::SetValue(int32 InValue, bool bBroadcast) {
  const int32 Clamped = FMath::Clamp(InValue, MinValue, FMath::Max(MinValue, MaxValue));
  const bool bChanged = Clamped != Value;
  Value = Clamped;
  ApplyVisualState();
  if (bChanged && bBroadcast) OnValueChanged.Broadcast(Value);
}

bool UDrQuantityStepper::Nudge(int32 Direction) {
  const int32 Before = Value;
  SetValue(Value + FMath::Sign(Direction) * Step, true);
  if (Value == Before) { HeldDirection = 0; return false; }
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Subsystem && Subsystem->IsReduceMotion()) Slide.Snap(0.f);
  else { Slide.Value = 10.f * FMath::Sign(Direction); Slide.Velocity = 0.f; Slide.Target = 0.f; }
  return true;
}

void UDrQuantityStepper::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (HeldDirection != 0) {
    HeldSeconds += DeltaTime;
    const float Interval = RepeatInterval(HeldSeconds);
    if (Interval > 0.f) {
      RepeatWait -= DeltaTime;
      if (RepeatWait <= 0.f) { RepeatWait = Interval; Nudge(HeldDirection); }
    }
  }
  if (!Slide.IsSettled() && ValueText) {
    Slide.Step(DeltaTime);
    ValueText->SetRenderTranslation(FVector2D(0.f, -Slide.Value));  // + 는 아래에서, − 는 위에서 들어온다
    ValueText->SetRenderOpacity(1.f - .5f * FMath::Min(1.f, FMath::Abs(Slide.Value) / 10.f));
  }
}

void UDrQuantityStepper::ApplyVisualState() {
  SetRenderOpacity(GetIsEnabled() ? 1.f : .4f);
  if (Surface) Surface->ApplyTheme();
  if (ValueText) { ValueText->SetTokens(TEXT("heading"), DrUITokens::Ink); ValueText->SetText(FText::AsNumber(Value)); }
  if (MinusButton) MinusButton->SetIsEnabled(Value > MinValue);
  if (PlusButton) PlusButton->SetIsEnabled(Value < MaxValue);
}
