#include "Widgets/DrText.h"
#include "Theme/DrUIThemeSubsystem.h"

void UDrText::SetTokens(FName InTextStyle, FName InColorToken) { TextStyle = InTextStyle; ColorToken = InColorToken; ApplyTheme(); }

void UDrText::ApplyTheme() {
  const UDrUITheme& Theme = UDrUIThemeSubsystem::Current();
  SetFont(Theme.MakeFont(TextStyle));
  SetColorAndOpacity(FSlateColor(Theme.GetColor(ColorToken)));
}

void UDrText::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrText::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Super::RebuildWidget();
}

void UDrText::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
