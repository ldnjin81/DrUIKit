#include "Flow/DrUIFlowSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Containers/Ticker.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Flow/DrUIRootLayout.h"
#include "Flow/DrUIScreens.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/App.h"
#include "Theme/DrUIKitSettings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/DrConfirmDialog.h"

UDrUIFlowSubsystem* UDrUIFlowSubsystem::Get(const UUserWidget* Widget) {
  const ULocalPlayer* Player = Widget ? Widget->GetOwningLocalPlayer() : nullptr;
  return Player ? Player->GetSubsystem<UDrUIFlowSubsystem>() : nullptr;
}

UDrUIFlowSubsystem* UDrUIFlowSubsystem::Get(const APlayerController* Controller) {
  const ULocalPlayer* Player = Controller ? Controller->GetLocalPlayer() : nullptr;
  return Player ? Player->GetSubsystem<UDrUIFlowSubsystem>() : nullptr;
}

void UDrUIFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
  Super::Initialize(Collection);
  if (ULocalPlayer* Player = GetLocalPlayer()) {
    ControllerHandle = Player->OnPlayerControllerChanged().AddUObject(this, &ThisClass::HandleControllerChanged);
    if (Player->PlayerController) HandleControllerChanged(Player->PlayerController);
  }
}

void UDrUIFlowSubsystem::Deinitialize() {
  if (ULocalPlayer* Player = GetLocalPlayer()) Player->OnPlayerControllerChanged().Remove(ControllerHandle);
  Teardown();
  Super::Deinitialize();
}

void UDrUIFlowSubsystem::HandleControllerChanged(APlayerController* Controller) {
  Teardown();
  if (!Controller || !GetDefault<UDrUIKitSettings>()->bAutoCreateRootLayout) return;
  // 컨트롤러가 막 정해진 때는 아직 BeginPlay 전이다. 다음 틱에 화면 층을 붙인다.
  TWeakObjectPtr<APlayerController> Weak(Controller);
  FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this, Weak](float) {
    if (APlayerController* PC = Weak.Get(); PC && GetLocalPlayer() && GetLocalPlayer()->PlayerController == PC && !Root) Setup(PC);
    return false;
  }));
}

void UDrUIFlowSubsystem::Setup(APlayerController* Controller) {
  Owner = Controller;
  const UDrUIKitSettings* Settings = GetDefault<UDrUIKitSettings>();
  if (FApp::CanEverRender()) {
    Root = CreateWidget<UDrUIRootLayout>(Controller, UDrUIRootLayout::StaticClass());
    if (Root) Root->AddToPlayerScreen(1000);  // 다른 화면 위젯보다 위(층 안에서 게임 → 메뉴 → 모달 → 알림)
  }
  // 일시정지 입력: 컨트롤러를 상속하지 않고 입력 컴포넌트를 하나 얹는다.
  if (UInputAction* Action = Settings->PauseAction.LoadSynchronous()) {
    PauseInput = NewObject<UEnhancedInputComponent>(Controller, TEXT("DrUIPauseInput"));
    PauseInput->BindAction(Action, ETriggerEvent::Started, this, &ThisClass::TogglePauseMenu);
    Controller->PushInputComponent(PauseInput);
  }
  PauseContext = Settings->MenuInputContext.LoadSynchronous();
  if (PauseContext)
    if (UEnhancedInputLocalPlayerSubsystem* Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) Input->AddMappingContext(PauseContext, 100);
  Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePawnChanged);
}

void UDrUIFlowSubsystem::Teardown() {
  APlayerController* Controller = Owner.Get();
  if (Root) Root->RemoveFromParent();
  if (Controller && PauseInput) Controller->PopInputComponent(PauseInput);
  if (PauseContext && GetLocalPlayer())
    if (UEnhancedInputLocalPlayerSubsystem* Input = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) Input->RemoveMappingContext(PauseContext);
  if (Controller) Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePawnChanged);
  Root = nullptr;
  HUD = nullptr;
  SettingsMenu = nullptr;
  PauseInput = nullptr;
  PauseContext = nullptr;
  Owner.Reset();
  bPauseOpen = bMenuInput = bTitle = bManagesInput = false;
  ExtraMenus = 0;
}

void UDrUIFlowSubsystem::ApplyInputMode(bool bMenu) {
  bMenuInput = bMenu;
  APlayerController* Controller = Owner.Get();
  if (!Controller || !bManagesInput) return;  // 게임이 맡기기 전에는 입력 모드를 건드리지 않는다(갤러리·자체 UI 보호)
  Controller->bShowMouseCursor = bMenu;
  if (bMenu) {
    FInputModeUIOnly Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Controller->SetInputMode(Mode);
  } else {
    Controller->SetInputMode(FInputModeGameOnly());
  }
}

void UDrUIFlowSubsystem::EnterTitle() {
  bTitle = true;
  bManagesInput = true;
  if (Root)
    if (UClass* Menu = GetDefault<UDrUIKitSettings>()->TitleMenuClass.LoadSynchronous()) { Root->ClearMenus(); Root->PushMenu(Menu); }
  ApplyInputMode(true);
}

void UDrUIFlowSubsystem::EnterGame() {
  bTitle = false;
  bManagesInput = true;
  if (Root && !HUD)
    if (UClass* HUDType = GetDefault<UDrUIKitSettings>()->HUDClass.LoadSynchronous()) HUD = Cast<UDrUIHUDWidget>(Root->PushGame(HUDType));
  ApplyInputMode(false);
  if (APlayerController* Controller = Owner.Get()) HandlePawnChanged(nullptr, Controller->GetPawn());
}

bool UDrUIFlowSubsystem::OpenPauseMenu() {
  APlayerController* Controller = Owner.Get();
  if (!Controller || bTitle || bPauseOpen || (CanOpenPauseMenu.IsBound() && !CanOpenPauseMenu.Execute())) return false;
  UClass* Menu = GetDefault<UDrUIKitSettings>()->PauseMenuClass.LoadSynchronous();
  if (!Menu || !Controller->SetPause(true)) return false;
  bPauseOpen = true;
  if (Root) Root->PushMenu(Menu);
  ApplyInputMode(true);
  OnMenuChanged.Broadcast(true);
  return true;
}

void UDrUIFlowSubsystem::ClosePauseMenu() {
  if (!bPauseOpen) return;
  SettingsMenu = nullptr;
  ExtraMenus = 0;
  if (Root) Root->ClearMenus();
  bPauseOpen = false;
  if (APlayerController* Controller = Owner.Get()) Controller->SetPause(false);
  ApplyInputMode(false);
  OnMenuChanged.Broadcast(false);
}

void UDrUIFlowSubsystem::TogglePauseMenu() {
  if (bPauseOpen) Back();
  else OpenPauseMenu();
}

void UDrUIFlowSubsystem::OpenSettings() {
  if (SettingsMenu || (!bPauseOpen && !bTitle) || !Root) return;
  if (UClass* Menu = GetDefault<UDrUIKitSettings>()->SettingsMenuClass.LoadSynchronous()) {
    SettingsMenu = Root->PushMenu(Menu);
    ApplyInputMode(true);
  }
}

void UDrUIFlowSubsystem::Back() {
  if (Root && Root->HasModal()) {  // 모달이 떠 있으면 그 창이 뒤로를 받는다(확인 대화상자는 취소)
    // HandleBackAction은 protected라: 확인 대화상자면 취소로 결정, 그 밖의 모달은 닫는다.
    if (UCommonActivatableWidget* Modal = Root->GetStack(EDrUILayer::Modal)->GetActiveWidget()) {
      if (UDrConfirmDialog* Dialog = Cast<UDrConfirmDialog>(Modal)) Dialog->Resolve(false);
      else Modal->DeactivateWidget();
    }
    return;
  }
  if (SettingsMenu) {
    if (Root) Root->PopMenu();
    SettingsMenu = nullptr;
  } else if (ExtraMenus > 0) {
    if (Root) Root->PopMenu();
    if (--ExtraMenus == 0 && !bPauseOpen && !bTitle) ApplyInputMode(false);
  } else if (!bTitle) {
    ClosePauseMenu();
  }
}

UCommonActivatableWidget* UDrUIFlowSubsystem::OpenMenu(TSubclassOf<UDrUIMenuWidget> MenuClass) {
  if (!Root || !MenuClass) return nullptr;
  UCommonActivatableWidget* Menu = Root->PushMenu(MenuClass);
  if (Menu) ++ExtraMenus;
  ApplyInputMode(true);
  return Menu;
}

UDrConfirmDialog* UDrUIFlowSubsystem::ShowConfirm(const FText& Title, const FText& Body, const FText& ConfirmLabel, const FText& CancelLabel, bool bDestructive, TFunction<void(bool)> Callback) {
  if (!Root) return nullptr;
  UCommonActivatableWidgetStack* Stack = Root->GetStack(EDrUILayer::Modal);
  if (!Stack) return nullptr;
  TSubclassOf<UDrConfirmDialog> DialogClass = GetDefault<UDrUIKitSettings>()->ConfirmDialogClass.LoadSynchronous();
  if (!DialogClass) DialogClass = UDrConfirmDialog::StaticClass();
  // 내용은 활성화 전에 넣는다(첫 포커스가 파괴적이면 취소로 가야 하므로).
  return Stack->AddWidget<UDrConfirmDialog>(DialogClass, [&](UDrConfirmDialog& Dialog) {
    Dialog.SetContent(Title, Body, ConfirmLabel, CancelLabel, bDestructive);
    Dialog.OnResultNative = MoveTemp(Callback);
  });
}

void UDrUIFlowSubsystem::ShowToast(const FText& Message, EDrNoticeTone Tone) {
  if (Root && Root->GetToasts()) Root->GetToasts()->Push(Message, Tone);
}

int32 UDrUIFlowSubsystem::ShowSnackbar(const FText& Message, const FText& ActionLabel) {
  return Root && Root->GetSnackbar() ? Root->GetSnackbar()->Show(Message, ActionLabel) : 0;
}

void UDrUIFlowSubsystem::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn) {
  if (HUD) HUD->OnOwnerPawnChanged(OldPawn, NewPawn);
}
