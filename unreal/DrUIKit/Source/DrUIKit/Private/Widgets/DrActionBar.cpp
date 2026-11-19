#include "Widgets/DrActionBar.h"
#include "Widgets/DrOptionRow.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/DrUIFeedback.h"
#include "Core/DrUIInput.h"
#include "Theme/DrUIThemeSubsystem.h"
#include "Widgets/DrButtonPrompt.h"
#include "Widgets/DrSurface.h"
#include "Widgets/DrText.h"

TSharedRef<SWidget> UDrActionBar::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    Surface = WidgetTree->ConstructWidget<UDrSurface>(UDrSurface::StaticClass(), TEXT("Surface"));
    Surface->RadiusToken = DrUITokens::RadiusLg;
    Surface->PaddingToken = NAME_None;
    Surface->SetPadding(FMargin(16.f, 8.f));
    WidgetTree->RootWidget = Surface;
    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Surface->SetContent(Row);
    StatusLabel = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("StatusLabel"));
    StatusLabel->SetTokens(DrUITokens::Label, DrUITokens::InkMuted);
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(StatusLabel)) { S->SetVerticalAlignment(VAlign_Center); S->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); }
    Prompts = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Prompts"));
    if (UHorizontalBoxSlot* S = Row->AddChildToHorizontalBox(Prompts)) S->SetVerticalAlignment(VAlign_Center);
  }
  if (Surface) {
    Surface->FillToken = bSurface ? DrUITokens::Glass : NAME_None;
    Surface->FillStrongToken = bSurface ? DrUITokens::GlassStrong : NAME_None;  // 없으면 흐림이 꺼진 표면이 glass-strong으로 칠해진다
    Surface->StrokeToken = bSurface ? DrUITokens::GlassStroke : NAME_None;
    Surface->BackdropToken = bSurface ? DrUITokens::BackdropGlass : NAME_None;
    Surface->ApplyTheme();
  }
  if (StatusLabel) StatusLabel->SetText(Status);
  ShownSignature.Reset();
  return Super::RebuildWidget();
}

void UDrActionBar::SetBaseHints(const TArray<FDrActionHint>& Hints) { BaseHints = Hints; ShownSignature.Reset(); }

void UDrActionBar::SetStatusText(const FText& Text) {
  Status = Text;
  if (StatusLabel) StatusLabel->SetText(Text);
}

TArray<FDrActionHint> UDrActionBar::CollectHints() const {
  TArray<FDrActionHint> Hints;
  if (bShowFocusHints) {
    // 옵션 줄 안이면 그 줄의 안내(좌우 조절 등)가 먼저, 그다음 포커스 버튼의 안내.
    if (const UDrOptionRow* Row = UDrOptionRow::GetFocusedRow(); Row && DrUIInput::IsInActiveLayer(Row)) Hints.Append(Row->ActionHints);
    if (const UDrButtonBase* Focused = UDrButtonBase::GetHintFocus(); Focused && DrUIInput::IsInActiveLayer(Focused)) Hints.Append(Focused->ActionHints);
  }
  for (const FDrActionHint& Base : BaseHints) {
    // 포커스 버튼이 같은 키를 쓰면 공통 안내는 뺀다(그 버튼의 뜻이 우선).
    const bool bTaken = Hints.ContainsByPredicate([&Base](const FDrActionHint& H) { return (Base.GamepadKey.IsValid() && H.GamepadKey == Base.GamepadKey) || (Base.KeyboardKey.IsValid() && H.KeyboardKey == Base.KeyboardKey); });
    if (!bTaken) Hints.Add(Base);
  }
  return Hints;
}

void UDrActionBar::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  // 포커스가 옮겨 가면 안내가 바뀐다. 바뀐 때만 다시 만든다.
  const TArray<FDrActionHint> Hints = CollectHints();
  FString Signature;
  for (const FDrActionHint& H : Hints) Signature += H.Id.ToString() + H.GamepadKey.ToString() + H.KeyboardKey.ToString() + H.Label.ToString() + TEXT("|");
  if (Signature != ShownSignature) { ShownSignature = Signature; Rebuild(Hints); }
}

void UDrActionBar::Rebuild(const TArray<FDrActionHint>& Hints) {
  if (!Prompts || !WidgetTree) return;
  Prompts->ClearChildren();
  for (const FDrActionHint& Hint : Hints) {
    UDrButtonPrompt* Prompt = WidgetTree->ConstructWidget<UDrButtonPrompt>();
    Prompt->SetKeys(Hint.KeyboardKey, Hint.GamepadKey, Hint.Label);
    Prompt->KeyHeight = 22.f;
    if (UHorizontalBoxSlot* S = Prompts->AddChildToHorizontalBox(Prompt)) { S->SetVerticalAlignment(VAlign_Center); S->SetPadding(FMargin(18, 0, 0, 0)); }
  }
}
