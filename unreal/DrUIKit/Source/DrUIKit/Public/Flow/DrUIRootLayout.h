#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DrUIRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;
class UDrSnackbar;
class UDrToastStack;

/** 화면 층(아래부터). */
UENUM(BlueprintType)
enum class EDrUILayer : uint8 {
  /** HUD 등 게임 화면 위 UI. */
  Game,
  /** 타이틀·일시정지·설정 메뉴. */
  Menu,
  /** 확인 대화상자 등 다른 것을 막는 창. */
  Modal
};

/**
 * 플레이어 화면의 층 구조: 게임 · 메뉴 · 모달(CommonUI 활성 위젯 스택) + 알림 층(토스트·스낵바). UI 흐름(UDrUIFlowSubsystem)이 로컬 플레이어마다 하나 만든다.
 * DrFramework 0.2.0의 UDrUIRootLayout(게임·메뉴 두 층)을 옮겨 모달·알림 층을 더했다.
 */
UCLASS()
class DRUIKIT_API UDrUIRootLayout : public UUserWidget {
  GENERATED_BODY()
public:
  UCommonActivatableWidget* Push(EDrUILayer Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass);
  UCommonActivatableWidget* PushGame(TSubclassOf<UCommonActivatableWidget> WidgetClass) { return Push(EDrUILayer::Game, WidgetClass); }
  UCommonActivatableWidget* PushMenu(TSubclassOf<UCommonActivatableWidget> WidgetClass) { return Push(EDrUILayer::Menu, WidgetClass); }
  /** 맨 위 메뉴를 닫는다. */
  void PopMenu();
  void ClearMenus();
  UCommonActivatableWidgetStack* GetStack(EDrUILayer Layer) const;
  bool HasModal() const;
  UDrToastStack* GetToasts() const { return Toasts; }
  UDrSnackbar* GetSnackbar() const { return Snackbar; }
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  void EnsureTree();
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidgetStack> GameStack;
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidgetStack> MenuStack;
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidgetStack> ModalStack;
  UPROPERTY(Transient) TObjectPtr<UDrToastStack> Toasts;
  UPROPERTY(Transient) TObjectPtr<UDrSnackbar> Snackbar;
};
