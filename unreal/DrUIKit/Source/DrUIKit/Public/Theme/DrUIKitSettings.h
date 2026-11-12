#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DrUIKitSettings.generated.h"

class UDrUITheme;

/** 표면(유리 등) 품질. 낮음은 뒤 흐림 없이 불투명 채움. */
UENUM(BlueprintType)
enum class EDrUISurfaceQuality : uint8 { Low, Medium, High };

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "DrUIKit"))
class DRUIKIT_API UDrUIKitSettings : public UDeveloperSettings {
  GENERATED_BODY()
public:
  /** 쓸 테마들. 비어 있으면 코어 회색조 기본 테마. */
  UPROPERTY(Config, EditAnywhere, Category = "테마") TArray<TSoftObjectPtr<UDrUITheme>> Themes;
  /** 시작 테마의 ThemeId. 비우면 Themes의 첫 번째. */
  UPROPERTY(Config, EditAnywhere, Category = "테마") FName StartTheme;
  UPROPERTY(Config, EditAnywhere, Category = "표면") EDrUISurfaceQuality SurfaceQuality = EDrUISurfaceQuality::High;
  /** 위치 이동을 페이드로 바꾸고 카운트 업 등을 끈다. */
  UPROPERTY(Config, EditAnywhere, Category = "움직임") bool bReduceMotion = false;
};
