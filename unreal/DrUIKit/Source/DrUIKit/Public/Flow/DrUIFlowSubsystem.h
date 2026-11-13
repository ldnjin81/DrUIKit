#pragma once
#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Widgets/DrNotice.h"
#include "DrUIFlowSubsystem.generated.h"

class APlayerController;
class UCommonActivatableWidget;
class UDrConfirmDialog;
class UDrUIHUDWidget;
class UDrUIMenuWidget;
class UDrUIRootLayout;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;

DECLARE_MULTICAST_DELEGATE_OneParam(FDrUIMenuChanged, bool /*bMenuOpen*/);

/**
 * UI 흐름(로컬 플레이어 서브시스템): 화면 층을 만들고 타이틀·HUD·일시정지 메뉴·설정·뒤로·입력 모드·확인 대화상자·알림을 맡는다.
 * 어떤 플레이어 컨트롤러를 써도 동작한다(컨트롤러 상속 불필요). 게임 흐름(맵 이동·재시작)은 게임 쪽 일이고, 일시정지는 컨트롤러의 SetPause만 부른다.
 * DrFramework 0.2.0 ADrPlayerController의 UI 흐름을 옮긴 것(2026-11-12 결정).
 */
UCLASS()
class DRUIKIT_API UDrUIFlowSubsystem : public ULocalPlayerSubsystem {
  GENERATED_BODY()
public:
  /** 위젯(소유 로컬 플레이어)·컨트롤러에서 찾기. */
  static UDrUIFlowSubsystem* Get(const UUserWidget* Widget);
  static UDrUIFlowSubsystem* Get(const APlayerController* Controller);

  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Deinitialize() override;

  /** 화면 층(없으면 지금 컨트롤러로 만든다). */
  UDrUIRootLayout* GetRootLayout() { EnsureSetup(); return Root; }
  UDrUIHUDWidget* GetHUD() const { return HUD; }
  bool IsMenuInput() const { return bMenuInput; }
  bool IsPauseMenuOpen() const { return bPauseOpen; }
  bool IsTitle() const { return bTitle; }

  /** 타이틀 화면: 설정의 TitleMenuClass를 메뉴 층에, 메뉴 입력. 일시정지 메뉴는 열리지 않는다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void EnterTitle();
  /** 게임 화면: 설정의 HUDClass를 게임 층에, 게임 입력. */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void EnterGame();
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") bool OpenPauseMenu();
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void ClosePauseMenu();
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void TogglePauseMenu();
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void OpenSettings();
  /** 뒤로: 모달이면 그 창의 뒤로, 설정이면 닫기, 일시정지면 재개(타이틀에서는 아무것도). */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void Back();
  /** 아무 메뉴나 메뉴 층에(메뉴 입력으로). */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") UCommonActivatableWidget* OpenMenu(TSubclassOf<UDrUIMenuWidget> MenuClass);
  /** 확인 대화상자를 모달 층에. Callback은 확인이면 true. */
  UDrConfirmDialog* ShowConfirm(const FText& Title, const FText& Body, const FText& ConfirmLabel, const FText& CancelLabel, bool bDestructive, TFunction<void(bool)> Callback = nullptr);
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") void ShowToast(const FText& Message, EDrNoticeTone Tone = EDrNoticeTone::Info);
  UFUNCTION(BlueprintCallable, Category = "DrUI|Flow") int32 ShowSnackbar(const FText& Message, const FText& ActionLabel);
  /** 메뉴 입력(UI만, 마우스 보임) ↔ 게임 입력. 게임이 EnterTitle·EnterGame으로 흐름에 맡긴 뒤에만 실제로 바꾼다(그 전에는 기록만). */
  void ApplyInputMode(bool bMenu);

  /** 게임이 일시정지를 막을 때(예: 컷신 중). 묶지 않으면 늘 허용. */
  TDelegate<bool()> CanOpenPauseMenu;
  /** 메뉴가 열리고 닫힐 때(게임이 사운드·시간을 맞출 때). */
  FDrUIMenuChanged OnMenuChanged;

private:
  void HandleControllerChanged(APlayerController* Controller);
  void Setup(APlayerController* Controller);
  /** 컨트롤러 변경 알림을 못 받았어도(맵 시작 경로에 따라 안 온다) 처음 쓸 때 화면 층을 만든다. */
  void EnsureSetup();
  void Teardown();
  UFUNCTION() void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);
  UPROPERTY(Transient) TObjectPtr<UDrUIRootLayout> Root;
  UPROPERTY(Transient) TObjectPtr<UDrUIHUDWidget> HUD;
  UPROPERTY(Transient) TObjectPtr<UCommonActivatableWidget> SettingsMenu;
  UPROPERTY(Transient) TObjectPtr<UEnhancedInputComponent> PauseInput;
  UPROPERTY(Transient) TObjectPtr<UInputMappingContext> PauseContext;
  TWeakObjectPtr<APlayerController> Owner;
  FDelegateHandle ControllerHandle;
  bool bMenuInput = false, bPauseOpen = false, bTitle = false;
  /** EnterTitle·EnterGame을 불러 입력 모드를 흐름이 맡았는지. */
  bool bManagesInput = false;
  /** OpenMenu로 연(일시정지·설정이 아닌) 메뉴 수. 뒤로가 하나씩 닫는다. */
  int32 ExtraMenus = 0;
};
