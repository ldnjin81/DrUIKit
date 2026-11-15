#include "Core/DrUIFeedback.h"
#include "Components/Widget.h"
#include "Core/DrUIInput.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Theme/DrUIKitSettings.h"
#include "Theme/DrUIThemeSubsystem.h"

void DrUIFeedback::HapticFor(EDrUIFeedback Kind, float& OutStrength, float& OutSeconds) {
  switch (Kind) {
    case EDrUIFeedback::Focus: OutStrength = .08f; OutSeconds = .03f; break;
    case EDrUIFeedback::Tick: OutStrength = .12f; OutSeconds = .03f; break;
    case EDrUIFeedback::Confirm: OutStrength = .3f; OutSeconds = .06f; break;
    case EDrUIFeedback::Back: OutStrength = .2f; OutSeconds = .05f; break;
    case EDrUIFeedback::Error: OutStrength = .55f; OutSeconds = .14f; break;
  }
}

void DrUIFeedback::Play(const UWidget* Context, EDrUIFeedback Kind) {
  if (!Context) return;
  const UDrUIKitSettings* Settings = GetDefault<UDrUIKitSettings>();
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (!Subsystem || Subsystem->IsUISounds()) {
    const TSoftObjectPtr<USoundBase>* Sound = Kind == EDrUIFeedback::Focus ? &Settings->FocusSound : Kind == EDrUIFeedback::Confirm ? &Settings->ConfirmSound
      : Kind == EDrUIFeedback::Back ? &Settings->BackSound : Kind == EDrUIFeedback::Error ? &Settings->ErrorSound : &Settings->TickSound;
    if (!Sound->IsNull()) if (USoundBase* Loaded = Sound->LoadSynchronous()) UGameplayStatics::PlaySound2D(Context, Loaded);
  }
  if ((Subsystem && !Subsystem->IsUIHaptics()) || Settings->HapticStrength <= 0.f || !DrUIInput::IsUsingGamepad(Context)) return;
  if (APlayerController* Player = Context->GetOwningPlayer()) {
    float Strength = 0.f, Seconds = 0.f;
    HapticFor(Kind, Strength, Seconds);
    // 에셋 없이 짧은 진동. 작은 모터만 써서 딸깍 하는 느낌으로.
    Player->PlayDynamicForceFeedback(Strength * Settings->HapticStrength, Seconds, false, true, false, true, EDynamicForceFeedbackAction::Start);
  }
}
