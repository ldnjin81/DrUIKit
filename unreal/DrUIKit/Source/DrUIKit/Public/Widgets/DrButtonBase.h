#pragma once
#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonInputTypeEnum.h"
#include "Core/DrUIMotion.h"
#include "Core/DrUIState.h"
#include "InputCoreTypes.h"
#include "DrButtonBase.generated.h"

/** 행동 안내 한 줄(행동 안내 바에 보일 "Ⓧ 버리기"). 버튼이 포커스를 받으면 안내 바가 이 목록을 보인다. */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrActionHint {
  GENERATED_BODY()
  /** 게임이 알아볼 이름(OnHintAction으로 돌아온다). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName Id;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey GamepadKey;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey KeyboardKey;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrOnHintAction, FName, HintId);

/** 그리지 않는 버튼 스타일. 모양은 DrUIKit 부품이 칠한다. */
UCLASS()
class DRUIKIT_API UDrButtonStyle : public UCommonButtonStyle {
  GENERATED_BODY()
public:
  UDrButtonStyle();
};

/**
 * 누르는 부품(버튼·체크박스·라디오·스위치·탭 버튼)의 공통 틀. CommonUI 버튼을 상속해 입력·포커스·패드·선택은 엔진이 맡고,
 * 여기서는 호버·눌림·포커스 등을 모아 상태 하나로 판정(DrUIState::Resolve)하고 테마가 바뀌면 다시 칠한다.
 * 파생은 BuildDefaultTree(기본 구성)와 ApplyVisualState(칠하기)만 채운다.
 *
 * 게임패드:
 * - 포커스 링은 패드·키보드로 옮긴 포커스에만(마우스로 눌러 생긴 포커스는 숨김, 그 뒤 패드를 쓰면 보임).
 * - 단축키(인라인 버튼 프롬프트): ShortcutGamepadKey·ShortcutKeyboardKey를 정하면 포커스가 다른 곳에 있어도 그 키로 눌린다
 *   (같은 층일 때: 덮개가 열려 있으면 덮개 안 버튼만). UDrButton은 버튼 안에 키 모양을 붙인다.
 * - 행동 안내(ActionHints): 포커스를 받으면 행동 안내 바가 보이고, 포커스 중 그 키를 누르면 OnHintAction.
 * - 반응: 패드·키보드로 포커스가 오면 Focus, 눌리면 Confirm 피드백(소리·진동).
 */
UCLASS(Abstract)
class DRUIKIT_API UDrButtonBase : public UCommonButtonBase {
  GENERATED_BODY()
public:
  UDrButtonBase(const FObjectInitializer& Initializer);

  EDrUIState GetVisualState() const { return VisualState; }
  virtual FDrUIStateInputs GetStateInputs() const;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") FKey ShortcutGamepadKey;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") FKey ShortcutKeyboardKey;
  /** 포커스를 받으면 행동 안내 바에 보일 행동들. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI|Gamepad") TArray<FDrActionHint> ActionHints;
  /** 포커스 중 ActionHints의 키를 눌렀거나 안내 바에서 그 행동을 눌렀을 때. */
  UPROPERTY(BlueprintAssignable, Category = "DrUI|Gamepad") FDrOnHintAction OnHintAction;

  /** 단축키를 정한다(없애려면 빈 키). */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Gamepad") void SetShortcut(FKey InKeyboardKey, FKey InGamepadKey);
  bool HasShortcut() const { return ShortcutGamepadKey.IsValid() || ShortcutKeyboardKey.IsValid(); }
  bool MatchesShortcut(const FKey& Key) const { return Key.IsValid() && (Key == ShortcutGamepadKey || Key == ShortcutKeyboardKey); }
  /** 단축키로 눌린 것처럼(누름 반응 + 클릭). */
  void TriggerShortcut();
  /** 행동 안내 하나를 실행(안내 바 클릭 등). */
  UFUNCTION(BlueprintCallable, Category = "DrUI|Gamepad") void TriggerHint(FName HintId);
  /** 지금 행동 안내를 보일 버튼(포커스가 든 DrUIKit 버튼, 없으면 nullptr). */
  static UDrButtonBase* GetHintFocus();
  /** 눌림·포커스 상태를 지운다(목록이 버튼을 빼서 다시 쓸 때: 포커스를 가진 채 빠지면 해제 알림이 오지 않는다). */
  void ResetInteractionState();

protected:
  /** 위젯 블루프린트가 루트를 두지 않았을 때 C++ 기본 구성을 만든다. */
  virtual void BuildDefaultTree() {}
  /** 상태가 바뀌거나 테마가 바뀌면 다시 칠한다. 게임은 이것만 덮어써도 모양을 바꿀 수 있다. */
  virtual void ApplyVisualState() {}
  /** 상태를 다시 판정하고 칠한다. */
  void RefreshState();
  /** 움직임을 써도 되는지. 움직임 줄이기가 켜졌거나 아직 한 번도 틱하지 않았으면(처음 보일 때) false: 스프링을 바로 맞춘다. */
  bool CanAnimate() const;
  /** 스프링 하나를 목표로 보낸다. 움직일 수 없으면 바로 맞춘다. */
  void DriveSpring(FDrUISpring& Spring, float Target) const;

  /** 눌렸을 때 크기(1이면 줄지 않음). */
  float PressedScale = .97f;
  bool bPressedNow = false, bFocusedNow = false;
  EDrUIState VisualState = EDrUIState::Default;
  FDrUISpring Scale;

  virtual void InitializeNativeClassData() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void NativeOnHovered() override;
  virtual void NativeOnUnhovered() override;
  virtual void NativeOnPressed() override;
  virtual void NativeOnReleased() override;
  virtual void NativeOnSelected(bool bBroadcast) override;
  virtual void NativeOnDeselected(bool bBroadcast) override;
  virtual void NativeOnEnabled() override;
  virtual void NativeOnDisabled() override;
  virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
  virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
  virtual void NativeOnClicked() override;
  virtual void NativeConstruct() override;
  virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
  /** 단축키가 바뀌면(버튼 안 키 모양 갱신용). */
  virtual void OnShortcutChanged() {}

  /** 마우스로 눌러 생긴 포커스인지(그러면 포커스 링을 숨긴다). */
  bool bFocusFromPointer = false;

private:
  void HandleInputMethodChanged(ECommonInputType InputType);
  FDelegateHandle ThemeHandle, InputHandle;
  bool bTicked = false;
};
