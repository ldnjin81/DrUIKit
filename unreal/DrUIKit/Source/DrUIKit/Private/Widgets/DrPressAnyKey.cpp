#include "Widgets/DrPressAnyKey.h"
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

float UDrPressAnyKey::PulseOpacity(float Seconds) {
  // 2.4초 주기로 1 → .35 → 1(사인), 너무 깜빡이지 않게 느리게.
  return .675f + .325f * FMath::Cos(2.f * PI * Seconds / 2.4f);
}

TSharedRef<SWidget> UDrPressAnyKey::RebuildWidget() {
  if (WidgetTree && !WidgetTree->RootWidget) {
    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
    WidgetTree->RootWidget = Column;
    Label = WidgetTree->ConstructWidget<UDrText>(UDrText::StaticClass(), TEXT("Label"));
    Label->SetTokens(TEXT("heading"), DrUITokens::Ink);
    Label->SetText(Text);
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Label)) S->SetHorizontalAlignment(HAlign_Center);
    Prompt = WidgetTree->ConstructWidget<UDrButtonPrompt>(UDrButtonPrompt::StaticClass(), TEXT("Prompt"));
    Prompt->Device = EDrPromptDevice::Both;
    Prompt->SetKeys(EKeys::Enter, EKeys::Gamepad_FaceButton_Bottom, FText::GetEmpty());
    if (UVerticalBoxSlot* S = Column->AddChildToVerticalBox(Prompt)) { S->SetHorizontalAlignment(HAlign_Center); S->SetPadding(FMargin(0, 10, 0, 0)); }
  }
  return Super::RebuildWidget();
}

void UDrPressAnyKey::SetText(const FText& InText) {
  Text = InText;
  if (Label) Label->SetText(InText);
}

void UDrPressAnyKey::ResetListening() { bTriggered = false; Shown = 0.f; }

void UDrPressAnyKey::NativeConstruct() {
  Super::NativeConstruct();
  Shown = 0.f;
  if (ListenerId) return;
  ListenerId = DrUIInput::AddAnyKeyListener([WeakThis = TWeakObjectPtr<UDrPressAnyKey>(this)](const FKey& Key, bool bGamepad) {
    UDrPressAnyKey* This = WeakThis.Get();
    if (!This || This->bTriggered || This->Shown < This->IgnoreSeconds || !DrUIInput::IsInActiveLayer(This) || !DrUIInput::IsShown(This)) return false;
    This->bTriggered = true;
    DrUIFeedback::Play(This, EDrUIFeedback::Confirm);
    This->OnPressed.Broadcast(Key, bGamepad);
    return This->bConsumeInput;
  });
}

void UDrPressAnyKey::NativeDestruct() {
  DrUIInput::RemoveAnyKeyListener(ListenerId);
  ListenerId = 0;
  Super::NativeDestruct();
}

void UDrPressAnyKey::NativeTick(const FGeometry& Geometry, float DeltaTime) {
  Super::NativeTick(Geometry, DeltaTime);
  Shown += DeltaTime;
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Label) Label->SetRenderOpacity(bTriggered || (Subsystem && Subsystem->IsReduceMotion()) ? 1.f : PulseOpacity(Shown));
}
