#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DrUIScreens.generated.h"

/**
 * 메뉴 기반(타이틀·일시정지·설정 등). 열리면 메뉴 입력(마우스 보임, UIInputContext 매핑), 첫 포커스는 FirstFocus,
 * 뒤로(Esc·패드 메뉴·CommonUI 뒤로)는 UI 흐름의 Back. DrFramework 0.2.0 UDrMenuWidget을 옮긴 것.
 */
UCLASS(Abstract)
class DRUIKIT_API UDrUIMenuWidget : public UCommonActivatableWidget {
  GENERATED_BODY()
public:
  UDrUIMenuWidget(const FObjectInitializer& Initializer);
  virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Focus") TObjectPtr<UWidget> FirstFocus;
protected:
  virtual void NativeOnActivated() override;
  virtual UWidget* NativeGetDesiredFocusTarget() const override;
  virtual bool NativeOnHandleBackAction() override;
  virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
};

/** HUD 기반. 게임 입력(마우스 잡음), 조종하는 폰이 바뀌면 OnOwnerPawnChanged. DrFramework 0.2.0 UDrHUDWidget을 옮긴 것. */
UCLASS(Abstract)
class DRUIKIT_API UDrUIHUDWidget : public UCommonActivatableWidget {
  GENERATED_BODY()
public:
  virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
  virtual void OnOwnerPawnChanged(APawn* OldPawn, APawn* NewPawn) {}
};
