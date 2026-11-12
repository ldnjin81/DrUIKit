#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DrGlassGalleryGameMode.generated.h"

/** 갤러리 테스트 레벨(L_DrUIGallery)의 게임 모드: 플레이어 없이 갤러리를 띄우고 마우스·키보드 UI 입력으로 둔다. */
UCLASS()
class DRUIGLASS_API ADrGlassGalleryGameMode : public AGameModeBase {
  GENERATED_BODY()
public:
  ADrGlassGalleryGameMode();
protected:
  virtual void BeginPlay() override;
};
