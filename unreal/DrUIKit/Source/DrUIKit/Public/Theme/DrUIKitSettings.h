#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DrUIKitSettings.generated.h"

class UDrUITheme;
class UDrConfirmDialog;
class UDrUIHUDWidget;
class UDrUIMenuWidget;
class UInputAction;
class UInputMappingContext;

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
  /** 표면 투명도: 0 불투명, .5 테마 그대로, 1 가장 투명. 테마의 TranslucentTokens(유리 채움)만 따른다. */
  UPROPERTY(Config, EditAnywhere, Category = "표면", meta = (ClampMin = 0, ClampMax = 1)) float SurfaceTransparency = .5f;
  /** 위치 이동을 페이드로 바꾸고 카운트 업 등을 끈다. */
  UPROPERTY(Config, EditAnywhere, Category = "움직임") bool bReduceMotion = false;

  // UI 흐름(UDrUIFlowSubsystem). DrFramework 0.2.0 UDrUISettings의 메뉴·HUD·UI 입력을 옮긴 것(맵 흐름은 DrFramework에 남음).
  /** 로컬 플레이어마다 화면 층을 자동으로 만든다(끄면 게임이 직접 만든다). */
  UPROPERTY(Config, EditAnywhere, Category = "흐름") bool bAutoCreateRootLayout = true;
  UPROPERTY(Config, EditAnywhere, Category = "흐름") TSoftClassPtr<UDrUIMenuWidget> TitleMenuClass;
  UPROPERTY(Config, EditAnywhere, Category = "흐름") TSoftClassPtr<UDrUIMenuWidget> PauseMenuClass;
  UPROPERTY(Config, EditAnywhere, Category = "흐름") TSoftClassPtr<UDrUIMenuWidget> SettingsMenuClass;
  UPROPERTY(Config, EditAnywhere, Category = "흐름") TSoftClassPtr<UDrUIHUDWidget> HUDClass;
  /** 비우면 코어 기본 확인 대화상자. */
  UPROPERTY(Config, EditAnywhere, Category = "흐름") TSoftClassPtr<UDrConfirmDialog> ConfirmDialogClass;
  /** 메뉴가 열려 있는 동안 쓰는 입력 매핑(메뉴 위젯이 켠다). */
  UPROPERTY(Config, EditAnywhere, Category = "입력") TSoftObjectPtr<UInputMappingContext> UIInputContext;
  /** 일시정지 열고 닫기 입력과 그 매핑(게임 중 늘 켜 둔다). */
  UPROPERTY(Config, EditAnywhere, Category = "입력") TSoftObjectPtr<UInputAction> PauseAction;
  UPROPERTY(Config, EditAnywhere, Category = "입력") TSoftObjectPtr<UInputMappingContext> MenuInputContext;
};
