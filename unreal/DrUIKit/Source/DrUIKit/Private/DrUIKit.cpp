#include "DrUIKit.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "Theme/DrUIKitSettings.h"
#include "Theme/DrUIStyle.h"
#include "Theme/DrUIThemeSubsystem.h"

DEFINE_LOG_CATEGORY(LogDrUIKit);

/** 엔진이 뜨면 켜진 스타일 플러그인(Tokens/style.json)의 테마를 모두 넣는다. */
class FDrUIKitModule : public IModuleInterface {
public:
  virtual void StartupModule() override {
    PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddLambda([] {
      UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
      if (Subsystem && GetDefault<UDrUIKitSettings>()->bLoadStylePlugins) DrUIStyle::LoadAllStyles(*Subsystem);
    });
  }
  virtual void ShutdownModule() override { FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle); }
private:
  FDelegateHandle PostEngineInitHandle;
};
IMPLEMENT_MODULE(FDrUIKitModule, DrUIKit)
