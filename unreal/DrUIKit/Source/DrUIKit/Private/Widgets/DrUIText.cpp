#include "Widgets/DrUIText.h"
#include "Theme/DrUIThemeSubsystem.h"

void UDrUIText::SetTokens(FName InTextStyle, FName InColorToken) { TextStyle = InTextStyle; ColorToken = InColorToken; ApplyTheme(); }

void UDrUIText::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  SetFont(Theme.MakeFont(TextStyle));
  SetColorAndOpacity(FSlateColor(Theme.GetColor(ColorToken)));
}

void UDrUIText::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrUIText::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Super::RebuildWidget();
}

void UDrUIText::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
