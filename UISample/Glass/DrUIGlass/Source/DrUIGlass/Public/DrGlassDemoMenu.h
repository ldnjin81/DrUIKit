#pragma once
#include "CoreMinimal.h"
#include "Flow/DrUIScreens.h"
#include "DrGlassDemoMenu.generated.h"

/** 갤러리용 메뉴 견본(UDrUIMenuWidget 파생): 유리 판 + 설명 + 닫기. UI 흐름의 메뉴 층·뒤로를 확인한다. 게임 메뉴도 이렇게 상속해 만든다. */
UCLASS()
class DRUIGLASS_API UDrGlassDemoMenu : public UDrUIMenuWidget {
  GENERATED_BODY()
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
};
