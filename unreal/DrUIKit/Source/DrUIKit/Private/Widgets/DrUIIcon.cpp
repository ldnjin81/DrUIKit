#include "Widgets/DrUIIcon.h"
#include "Theme/DrUIThemeSubsystem.h"

void UDrUIIcon::ApplyTheme() {
  SetColorAndOpacity(UDrUIThemeSubsystem::Current().GetColor(ColorToken));
  SetDesiredSizeOverride(FVector2D(IconSize));
}

void UDrUIIcon::SynchronizeProperties() { ApplyTheme(); Super::SynchronizeProperties(); }

TSharedRef<SWidget> UDrUIIcon::RebuildWidget() {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get(); Subsystem && !ThemeHandle.IsValid())
    ThemeHandle = Subsystem->OnThemeChanged.AddWeakLambda(this, [this](const UDrUITheme&) { ApplyTheme(); });
  return Super::RebuildWidget();
}

void UDrUIIcon::ReleaseSlateResources(bool bReleaseChildren) {
  if (UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get()) Subsystem->OnThemeChanged.Remove(ThemeHandle);
  ThemeHandle.Reset();
  Super::ReleaseSlateResources(bReleaseChildren);
}
