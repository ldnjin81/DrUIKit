#include "Widgets/DrTooltip.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

namespace {
bool IsTooltipReduceMotion() {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  return Subsystem && Subsystem->IsReduceMotion();
}
}

UDrTooltip* UDrTooltip::Attach(UUserWidget* Owner, UWidget* Target, const FText& InTitle, const FText& InBody, TSubclassOf<UDrTooltip> TooltipClass) {
  if (!Owner || !Target) return nullptr;
  UDrTooltip* Tooltip = CreateWidget<UDrTooltip>(Owner, TooltipClass ? TooltipClass.Get() : UDrTooltip::StaticClass());
  if (!Tooltip) return nullptr;
  Tooltip->SetContent(InTitle, InBody);
  Target->SetToolTip(Tooltip);
  return Tooltip;
}

void UDrTooltip::SetContent(const FText& InTitle, const FText& InBody) {
  Title = InTitle;
  Body = InBody;
  ApplyTheme();
}

TSharedRef<SWidget> UDrTooltip::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    WidgetTree->RootWidget = Frame;
    Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
    Surface->FillToken = DrUITokens::GlassStrong;
    Surface->BackdropToken = DrUITokens::BackdropSoft;  // 디자인 시스템: 툴팁·토스트는 약한 흐림
    Surface->StrokeToken = DrUITokens::GlassStroke;
    Surface->RadiusToken = DrUITokens::RadiusSm;
    Surface->PaddingToken = NAME_None;
    Surface->SetPadding(FMargin(10.f, 7.f));
    Frame->AddChild(Surface);
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    Surface->SetContent(Column);
    TitleText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("TitleText"));
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(TitleText)) S->SetPadding(FMargin(0, 0, 0, 2));
    BodyText = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("BodyText"));
    BodyText->SetAutoWrapText(true);  // 크기 상자의 최대 너비 안에서 줄바꿈
    Column->AddChildToVerticalBox(BodyText);
    ApplyTheme();
  }
  SetVisibility(ESlateVisibility::HitTestInvisible);
  return Super::RebuildWidget();
}

void UDrTooltip::NativeOnInitialized() {
  Super::NativeOnInitialized();
  Presence.Stiffness = 420.f;
  Presence.Damping = 36.f;
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrTooltip::NativeConstruct() {
  Super::NativeConstruct();
  // 툴팁 창이 뜰 때마다 다시 만들어진다(Construct). 그때마다 위에서 살짝 내려오며 나타난다.
  if (IsTooltipReduceMotion()) Presence.Snap(1.f);
  else { Presence.Snap(0.f); Presence.Target = 1.f; }
  ApplyPresence();
}

void UDrTooltip::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::NativeDestruct();
}

void UDrTooltip::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!Presence.IsSettled()) {
    if (IsTooltipReduceMotion()) Presence.Snap(Presence.Target);
    else Presence.Step(DeltaTime);
    ApplyPresence();
  }
}

void UDrTooltip::ApplyPresence() {
  SetRenderOpacity(FMath::Clamp(Presence.Value, 0.f, 1.f));
  SetRenderTranslation(FVector2D(0.f, -4.f * (1.f - Presence.Value)));
}

void UDrTooltip::ApplyTheme() {
  if (Frame) Frame->SetMaxDesiredWidth(MaxWidth);
  if (Surface) Surface->ApplyTheme();
  if (TitleText) {
    TitleText->SetTokens(DrUITokens::Label, DrUITokens::Ink);
    TitleText->SetText(Title);
    TitleText->SetVisibility(Title.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
  if (BodyText) {
    // 제목이 있으면 설명은 보조 글(ink-muted), 설명만 있으면 본문색.
    BodyText->SetTokens(DrUITokens::Caption, Title.IsEmpty() ? DrUITokens::Ink : DrUITokens::InkMuted);
    BodyText->SetText(Body);
    BodyText->SetVisibility(Body.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  }
}
