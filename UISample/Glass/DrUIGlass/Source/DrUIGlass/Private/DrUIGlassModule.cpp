#include "DrGlassTheme.h"
#include "DrUIKit.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "Theme/DrUIThemeSubsystem.h"

/** 엔진이 뜨면 Glass 테마 두 개를 등록한다(밝은 유리가 먼저라 기본이 된다). */
class FDrUIGlassModule : public IModuleInterface {
public:
  virtual void StartupModule() override {
    FCoreDelegates::OnPostEngineInit.AddLambda([] {
      UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
      if (!Subsystem) return;
      FString Error;
      for (UDrUITheme* Theme : DrGlassTheme::LoadThemes(Subsystem, Error)) Subsystem->AddTheme(Theme);
      if (!Error.IsEmpty()) UE_LOG(LogDrUIKit, Error, TEXT("DrUIGlass: %s"), *Error);
    });
  }
};
IMPLEMENT_MODULE(FDrUIGlassModule, DrUIGlass)
