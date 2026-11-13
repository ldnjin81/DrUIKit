#include "Widgets/DrCountUp.h"
#include "Theme/DrUIThemeSubsystem.h"

void UDrCountUp::CountTo(float Target) {
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Subsystem && Subsystem->IsReduceMotion()) SetCurrentValue(Target);
  else InterpolateToValue(Target, CountSeconds, 0.f, 0.f);
}

void UDrCountUp::SetTokens(FName InTextStyle, FName InColorToken) { TextStyle = InTextStyle; ColorToken = InColorToken; ApplyTheme(); }

void UDrCountUp::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  SetFont(Theme.MakeFont(TextStyle));
  SetColorAndOpacity(FSlateColor(Theme.GetColor(ColorToken)));
}

void UDrCountUp::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrCountUp::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Super::RebuildWidget();
}

void UDrCountUp::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
