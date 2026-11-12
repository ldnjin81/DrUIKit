#include "Theme/DrUIThemeSubsystem.h"
#include "Engine/Engine.h"

namespace { UDrUITheme* GStandaloneDefault = nullptr; }

UDrUIThemeSubsystem* UDrUIThemeSubsystem::Get() { return GEngine ? GEngine->GetEngineSubsystem<UDrUIThemeSubsystem>() : nullptr; }

const UDrUITheme& UDrUIThemeSubsystem::Current() {
  if (const UDrUIThemeSubsystem* Subsystem = Get()) return Subsystem->GetTheme();
  if (!GStandaloneDefault) { GStandaloneDefault = UDrUITheme::MakeDefault(nullptr); GStandaloneDefault->AddToRoot(); }
  return *GStandaloneDefault;
}

void UDrUIThemeSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
  const UDrUIKitSettings* Settings = GetDefault<UDrUIKitSettings>();
  Fallback = UDrUITheme::MakeDefault(this);
  for (const TSoftObjectPtr<UDrUITheme>& Soft : Settings->Themes)
    if (UDrUITheme* Theme = Soft.LoadSynchronous()) Themes.Add(Theme);
  SurfaceQuality = Settings->SurfaceQuality;
  bReduceMotion = Settings->bReduceMotion;
  Active = Themes.IsEmpty() ? Fallback.Get() : Themes[0].Get();
  if (!Settings->StartTheme.IsNone()) SetTheme(Settings->StartTheme);
}

const UDrUITheme& UDrUIThemeSubsystem::GetTheme() const { return Active ? *Active : *Fallback; }

bool UDrUIThemeSubsystem::SetTheme(FName ThemeId) {
  for (UDrUITheme* Theme : Themes) {
    if (Theme && Theme->ThemeId == ThemeId) { Active = Theme; Broadcast(); return true; }
  }
  if (Fallback && Fallback->ThemeId == ThemeId) { Active = Fallback; Broadcast(); return true; }
  return false;
}

void UDrUIThemeSubsystem::SetThemeObject(UDrUITheme* Theme) {
  if (!Theme) return;
  Themes.AddUnique(Theme);
  Active = Theme;
  Broadcast();
}

void UDrUIThemeSubsystem::AddTheme(UDrUITheme* Theme) {
  if (!Theme) return;
  Themes.RemoveAll([Theme](const TObjectPtr<UDrUITheme>& Existing) { return Existing && Existing->ThemeId == Theme->ThemeId; });
  Themes.Add(Theme);
  if (Active == Fallback) { Active = Theme; Broadcast(); }
}

TArray<FName> UDrUIThemeSubsystem::GetThemeIds() const {
  TArray<FName> Ids;
  for (const UDrUITheme* Theme : Themes) if (Theme) Ids.Add(Theme->ThemeId);
  if (Ids.IsEmpty() && Fallback) Ids.Add(Fallback->ThemeId);
  return Ids;
}

void UDrUIThemeSubsystem::SetSurfaceQuality(EDrUISurfaceQuality Quality) { SurfaceQuality = Quality; Broadcast(); }

void UDrUIThemeSubsystem::Broadcast() { OnThemeChanged.Broadcast(GetTheme()); }
