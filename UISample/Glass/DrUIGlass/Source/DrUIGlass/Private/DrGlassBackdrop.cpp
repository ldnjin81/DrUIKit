#include "DrGlassBackdrop.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Theme/DrUIThemeSubsystem.h"

namespace {
struct FBlob { FName Token; FVector2D Anchor; float Size; };
const FBlob BlobLayout[] = {{TEXT("backdrop-a"), {.1, .05}, 1300.f}, {TEXT("backdrop-b"), {.92, .95}, 1200.f}, {TEXT("backdrop-c"), {.6, .3}, 800.f}};
// 번진 색: 크기만 줄인 둥근 원을 낮은 투명도로 겹쳐 가운데는 진하고 가장자리는 사라지게 한다(텍스처·실시간 흐림 없음).
constexpr int32 Rings = 20;
constexpr float CenterOpacity = .6f;
}

TSharedRef<SWidget> UDrGlassBackdrop::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    Ground = WidgetTree->ConstructWidget<UBorder>();
    if (UCanvasPanelSlot* GroundSlot = Canvas->AddChildToCanvas(Ground)) { GroundSlot->SetAnchors(FAnchors(0, 0, 1, 1)); GroundSlot->SetOffsets(FMargin(0)); }
    for (const FBlob& Blob : BlobLayout)
      for (int32 Ring = 0; Ring < Rings; ++Ring) {
        UBorder* Circle = WidgetTree->ConstructWidget<UBorder>();
        if (UCanvasPanelSlot* CircleSlot = Canvas->AddChildToCanvas(Circle)) {
          CircleSlot->SetAnchors(FAnchors(Blob.Anchor.X, Blob.Anchor.Y));
          CircleSlot->SetAlignment(FVector2D(.5, .5));
          CircleSlot->SetSize(FVector2D(Blob.Size * (1.f - float(Ring) / Rings)));
        }
        Blobs.Add(Circle);
      }
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
  // 겹친 원 Rings개의 투명도 합이 가운데에서 CenterOpacity가 되도록 한 겹의 투명도를 정한다.
  const float RingOpacity = 1.f - FMath::Pow(1.f - CenterOpacity, 1.f / Rings);
  for (int32 I = 0; I < Blobs.Num(); ++I) {
    const FBlob& Blob = BlobLayout[I / Rings];
    const float Size = Blob.Size * (1.f - float(I % Rings) / Rings);
    if (Blobs[I]) Blobs[I]->SetBrush(FSlateRoundedBoxBrush(Theme.GetColor(Blob.Token).CopyWithNewOpacity(RingOpacity), Size * .5f));
  }
}
