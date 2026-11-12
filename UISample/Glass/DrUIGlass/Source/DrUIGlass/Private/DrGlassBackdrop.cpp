#include "DrGlassBackdrop.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Theme/DrUIThemeSubsystem.h"

namespace {
struct FBlob { FName Token; FVector2D Anchor; float Size; };
const FBlob BlobLayout[] = {{TEXT("backdrop-a"), {.12, .1}, 620.f}, {TEXT("backdrop-b"), {.88, .9}, 560.f}, {TEXT("backdrop-c"), {.62, .25}, 380.f}};
}

TSharedRef<SWidget> UDrGlassBackdrop::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    Ground = WidgetTree->ConstructWidget<UBorder>();
    if (UCanvasPanelSlot* GroundSlot = Canvas->AddChildToCanvas(Ground)) { GroundSlot->SetAnchors(FAnchors(0, 0, 1, 1)); GroundSlot->SetOffsets(FMargin(0)); }
    for (const FBlob& Blob : BlobLayout) {
      UBorder* Circle = WidgetTree->ConstructWidget<UBorder>();
      if (UCanvasPanelSlot* CircleSlot = Canvas->AddChildToCanvas(Circle)) {
        CircleSlot->SetAnchors(FAnchors(Blob.Anchor.X, Blob.Anchor.Y));
        CircleSlot->SetAlignment(FVector2D(.5, .5));
        CircleSlot->SetSize(FVector2D(Blob.Size));
      }
      Blobs.Add(Circle);
    }
    // 원을 그대로 두면 또렷한 도형으로 보인다. 화면 전체를 아주 강하게 흐려 색이 번진 바탕(메시 그라데이션)으로 만든다.
    UBackgroundBlur* Wash = WidgetTree->ConstructWidget<UBackgroundBlur>();
    Wash->SetBlurStrength(100.f);
    if (UCanvasPanelSlot* WashSlot = Canvas->AddChildToCanvas(Wash)) { WashSlot->SetAnchors(FAnchors(0, 0, 1, 1)); WashSlot->SetOffsets(FMargin(0)); }
    ApplyTheme();
  }
  return Super::RebuildWidget();
}

void UDrGlassBackdrop::NativeOnInitialized() {
  Super::NativeOnInitialized();
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
}

void UDrGlassBackdrop::NativeDestruct() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  Super::NativeDestruct();
}

void UDrGlassBackdrop::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  if (Ground) Ground->SetBrush(FSlateColorBrush(Theme.GetColor(TEXT("ground"))));
  for (int32 I = 0; I < Blobs.Num(); ++I)
    if (Blobs[I]) Blobs[I]->SetBrush(FSlateRoundedBoxBrush(Theme.GetColor(BlobLayout[I].Token), BlobLayout[I].Size * .5f));
}
