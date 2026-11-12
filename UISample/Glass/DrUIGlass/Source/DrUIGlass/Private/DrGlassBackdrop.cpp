#include "DrGlassBackdrop.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Theme/DrUIThemeSubsystem.h"

namespace {
/** 가운데가 진하고 가장자리로 부드럽게 사라지는 흰 원(알파만). 색은 브러시 틴트로 입힌다. 한 번 만들어 공유. */
UTexture2D* SoftDisc() {
  static TStrongObjectPtr<UTexture2D> Shared;
  if (Shared) return Shared.Get();
  constexpr int32 Size = 256;
  UTexture2D* Texture = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
  if (!Texture) return nullptr;
  FColor* Pixels = static_cast<FColor*>(Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
  for (int32 Y = 0; Y < Size; ++Y)
    for (int32 X = 0; X < Size; ++X) {
      const float D = FVector2D(X + .5f - Size * .5f, Y + .5f - Size * .5f).Size() / (Size * .5f);
      const float A = FMath::SmoothStep(1.f, 0.f, D);  // 1 → 0 부드럽게
      Pixels[Y * Size + X] = FColor(255, 255, 255, uint8(255.f * A * A));
    }
  Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
  Texture->SRGB = false;
  Texture->UpdateResource();
  Shared.Reset(Texture);
  return Texture;
}

struct FBlob { FName Token; FVector2D Anchor; float Size; };
const FBlob BlobLayout[] = {{TEXT("backdrop-a"), {.12, .08}, 1300.f}, {TEXT("backdrop-b"), {.9, .92}, 1200.f}, {TEXT("backdrop-c"), {.6, .3}, 800.f}};
}

TSharedRef<SWidget> UDrGlassBackdrop::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    Ground = WidgetTree->ConstructWidget<UBorder>();
    if (UCanvasPanelSlot* GroundSlot = Canvas->AddChildToCanvas(Ground)) { GroundSlot->SetAnchors(FAnchors(0, 0, 1, 1)); GroundSlot->SetOffsets(FMargin(0)); }
    for (const FBlob& Blob : BlobLayout) {
      UImage* Circle = WidgetTree->ConstructWidget<UImage>();
      if (UCanvasPanelSlot* CircleSlot = Canvas->AddChildToCanvas(Circle)) {
        CircleSlot->SetAnchors(FAnchors(Blob.Anchor.X, Blob.Anchor.Y));
        CircleSlot->SetAlignment(FVector2D(.5, .5));
        CircleSlot->SetSize(FVector2D(Blob.Size));
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
  for (int32 I = 0; I < Blobs.Num(); ++I)
    if (Blobs[I]) {
      // 실시간 흐림(비쌈) 대신 가장자리가 사라지는 원 텍스처에 색을 입혀 번진 바탕을 만든다.
      FSlateBrush Brush;
      Brush.SetResourceObject(SoftDisc());
      Brush.ImageSize = FVector2D(BlobLayout[I].Size);
      Brush.TintColor = FSlateColor(Theme.GetColor(BlobLayout[I].Token));
      Blobs[I]->SetBrush(Brush);
    }
}
