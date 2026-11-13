#include "Widgets/DrMorphButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrGlyph.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

UDrMorphButton::UDrMorphButton(const FObjectInitializer& Initializer) : Super(Initializer) {
  Morph.Snap(0.f);
  CheckDraw.Snap(0.f);
}

void UDrMorphButton::BuildDefaultTree() {
  Super::BuildDefaultTree();
  if (!Surface) return;
  // 표면을 너비 상자로 감싸 줄였다 편다. 모핑 버튼은 알약(원이 되므로).
  MorphBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MorphBox"));
  WidgetTree->RootWidget = MorphBox;
  MorphBox->AddChild(Surface);
  MorphBox->SetClipping(EWidgetClipping::ClipToBounds);
  Surface->RadiusToken = DrUITokens::RadiusPill;
  if (UHorizontalBox* Row = Cast<UHorizontalBox>(Surface->GetContent())) {
    Check = WidgetTree->ConstructWidget<UDrGlyph>(UDrGlyph::StaticClass(), TEXT("Check"));
    Check->Shape = EDrGlyphShape::Check;
    Check->GlyphSize = 18.f;
    Check->SetVisibility(ESlateVisibility::Collapsed);
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Check)) S->SetVerticalAlignment(VAlign_Center);  // 원 상태에서는 글이 접혀 체크만 남는다
  }
}

void UDrMorphButton::ApplyVisualState() {
  Super::ApplyVisualState();
  if (VisualState != LastState) {
    if (VisualState == EDrUIState::Success) { SuccessAge = 0.f; CheckDraw.Snap(0.f); CheckDraw.Target = 1.f; }
    LastState = VisualState;
  }
  const bool bReduce = UDrUIThemeSubsystem::Get() && UDrUIThemeSubsystem::Get()->IsReduceMotion();
  // 로딩 중이거나 성공 직후(체크를 그리는 동안)는 원.
  const bool bRound = !bReduce && (VisualState == EDrUIState::Loading || (VisualState == EDrUIState::Success && SuccessAge >= 0.f && SuccessAge < CheckSeconds));
  DriveSpring(Morph, bRound ? 1.f : 0.f);
  if (bReduce) CheckDraw.Snap(1.f);
  ApplyMorph();
}

void UDrMorphButton::ApplyMorph() {
  if (!MorphBox || !Surface) return;
  const float Amount = FMath::Clamp(Morph.Value, 0.f, 1.f);
  if (Amount <= 0.f && Morph.IsSettled()) {
    MorphBox->ClearWidthOverride();
    FullWidth = Surface->GetDesiredSize().X;  // 펴진 너비를 재어 둔다(글이 바뀌어도 맞게)
  } else {
    const float Height = Surface->GetDesiredSize().Y;
    if (FullWidth <= 0.f) FullWidth = Surface->GetDesiredSize().X;
    MorphBox->SetWidthOverride(FMath::Lerp(FMath::Max(FullWidth, Height), Height, Amount));
  }
  // 줄어드는 동안 글은 먼저 사라지고, 원 상태에서는 스피너 또는 체크만.
  const bool bRoundNow = Amount > .35f;
  if (LabelText) LabelText->SetVisibility(bRoundNow ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
  if (Check) {
    const bool bShowCheck = bRoundNow && VisualState == EDrUIState::Success;
    Check->SetVisibility(bShowCheck ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    if (bShowCheck) { Check->SetColorToken(LabelText ? LabelText->ColorToken : DrUITokens::OnAccent); Check->SetProgress(CheckDraw.Value); }
  }
}

void UDrMorphButton::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  bool bChanged = false;
  if (SuccessAge >= 0.f) {
    const bool bWasChecking = SuccessAge < CheckSeconds;
    SuccessAge += DeltaTime;
    if (bWasChecking && SuccessAge >= CheckSeconds) ApplyVisualState();  // 체크를 다 보였으면 편다
    if (VisualState != EDrUIState::Success) SuccessAge = -1.f;
  }
  if (!Morph.IsSettled()) { Morph.Step(DeltaTime); bChanged = true; }
  if (!CheckDraw.IsSettled()) { CheckDraw.Step(DeltaTime); bChanged = true; }
  if (bChanged) ApplyMorph();
}
