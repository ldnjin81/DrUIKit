#include "DrUIKit.h"
#include "Misc/CoreDelegates.h"
#include "Core/DrUIInput.h"
#include "Modules/ModuleManager.h"
#include "Theme/DrUIKitSettings.h"
#include "Theme/DrUIGlyphImages.h"
#include "Theme/DrUIStyle.h"
#include "Theme/DrUIThemeSubsystem.h"

DEFINE_LOG_CATEGORY(LogDrUIKit);

/** 엔진이 뜨면 켜진 스타일 플러그인(Tokens/style.json)의 테마와 버튼 이미지 세트(Glyphs/glyphs.json)를 모두 넣고, 게임패드 UI 전역 입력 처리기(DrUIInput)를 붙인다. */
class FDrUIKitModule : public IModuleInterface {
public:
  virtual void StartupModule() override {
    PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddLambda([] {
      UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
      if (Subsystem && GetDefault<UDrUIKitSettings>()->bLoadStylePlugins) DrUIStyle::LoadAllStyles(*Subsystem);
      DrUIGlyphImages::LoadAll();
      DrUIInputProcessor::Register();
    });
  }
  virtual void ShutdownModule() override {
    FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
    DrUIInputProcessor::Unregister();
  }
private:
  FDelegateHandle PostEngineInitHandle;
};
IMPLEMENT_MODULE(FDrUIKitModule, DrUIKit)
