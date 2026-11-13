#include "Widgets/DrHoldButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrHoldButton::UDrHoldButton(const FObjectInitializer& Initializer) : Super(Initializer) {
  bRequiresHold = true;
  PressedScale = .98f;
}

void UDrHoldButton::SetHoldSeconds(float Seconds) {
  HoldSeconds = FMath::Max(.05f, Seconds);
  ApplyHoldTimes();
}

void UDrHoldButton::ApplyHoldTimes() {
  HoldTime = HoldSeconds;
  HoldRollbackTime = RollbackSeconds;
}

void UDrHoldButton::UpdateHoldData(ECommonInputType CurrentInputType) {
  Super::UpdateHoldData(CurrentInputType);  // 설정의 기본 HoldData 값이 들어온다
  ApplyHoldTimes();                          // 이 부품의 값으로 덮어쓴다
}

void UDrHoldButton::NativeOnInitialized() {
  Super::NativeOnInitialized();
  ApplyHoldTimes();
}

void UDrHoldButton::BuildDefaultTree() {
  Super::BuildDefaultTree();
  if (!Surface) return;
  // 버튼 표면 안을 [채움, 글줄] 겹으로 바꾼다. 채움이 표면 끝까지 닿도록 여백은 글줄 쪽으로 옮긴다.
  UWidget* Row = Surface->GetContent();
  Surface->SetPadding(FMargin(0.f));
  Surface->SetHorizontalAlignment(HAlign_Fill);
  Surface->SetVerticalAlignment(VAlign_Fill);
  UOverlay* Layers = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Layers"));
  Surface->SetContent(Layers);
  FillBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FillBox"));
  Fill = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Fill"));
  Fill->FillStrongToken = NAME_None;
  Fill->StrokeToken = NAME_None;
  Fill->BackdropToken = NAME_None;
  Fill->PaddingToken = NAME_None;
  Fill->RadiusToken = DrUITokens::RadiusSm;
  FillBox->AddChild(Fill);
  FillBox->SetVisibility(ESlateVisibility::Collapsed);
  if (UOverlaySlot* S = Layers->AddChildToOverlay(FillBox)) { S->SetHorizontalAlignment(HAlign_Left); S->SetVerticalAlignment(VAlign_Fill); }
  if (Row) if (UOverlaySlot* S = Layers->AddChildToOverlay(Row)) { S->SetHorizontalAlignment(HAlign_Center); S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(16.f, 10.f)); }
}

void UDrHoldButton::ApplyVisualState() {
  Super::ApplyVisualState();
  if (Fill && LabelText) {
    Fill->FillToken = LabelText->ColorToken;  // 글자색을 옅게: Primary면 흰빛, Secondary면 먹빛
    Fill->ApplyTheme();
    Fill->SetRenderOpacity(FillOpacity);
  }
}

void UDrHoldButton::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  if (!FillBox || !Surface) return;
  const float Progress = FMath::Clamp(CurrentHoldProgress, 0.f, 1.f);
  FillBox->SetVisibility(Progress > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
  if (Progress > 0.f) FillBox->SetWidthOverride(Progress * Surface->GetCachedGeometry().GetLocalSize().X);
}
