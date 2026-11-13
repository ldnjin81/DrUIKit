#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonInputTypeEnum.h"
#include "InputCoreTypes.h"
#include "DrButtonPrompt.generated.h"

class UDrProgressBar;
class UDrSurface;
class UDrText;
class USizeBox;

UENUM(BlueprintType)
enum class EDrPromptDevice : uint8 {
  /** 지금 쓰는 입력 장치를 따라 바뀐다(CommonInput). */
  Auto,
  Keyboard,
  Gamepad
};

/**
 * 버튼 프롬프트("[A] 확인", "[Esc] 뒤로"). 키를 키캡 칩(키보드는 각진 칩, 패드 얼굴 버튼은 원)으로 그리고 옆에 행동 이름을 붙인다.
 * 키보드·패드 키를 따로 정해 두면 마지막으로 쓴 입력 장치에 맞춰 바뀐다. 아이콘 그림 에셋 없이 글자로만 그린다
 * (엔진 UCommonActionWidget은 컨트롤러 데이터 에셋의 아이콘이 있어야 보인다 — 아이콘이 필요하면 그쪽을 쓴다).
 * SetHoldProgress로 길게 누르기 진행을 키캡 아래 얇은 막대로 보인다.
 */
UCLASS()
class DRUIKIT_API UDrButtonPrompt : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey KeyboardKey = EKeys::Enter;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey GamepadKey = EKeys::Gamepad_FaceButton_Bottom;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrPromptDevice Device = EDrPromptDevice::Auto;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetKeys(FKey InKeyboardKey, FKey InGamepadKey, const FText& InLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetDevice(EDrPromptDevice InDevice);
  /** 0이면 막대 숨김. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetHoldProgress(float Progress);

  /** 키캡에 쓸 짧은 이름(패드 A·B·X·Y·LB·RT·LS·←, 키보드 Esc·Enter·Space·Tab·F1 등). 모르는 키는 엔진 표시 이름. */
  static FText ShortKeyName(const FKey& Key);
  /** 원 모양 키캡을 쓸 키(패드 얼굴 버튼). */
  static bool IsRoundKey(const FKey& Key);
  /** 지금 보일 키. */
  FKey GetShownKey() const;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> CapBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Cap;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> KeyText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrProgressBar> HoldBar;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void ApplyTheme();
private:
  void HandleInputMethodChanged(ECommonInputType InputType);
  ECommonInputType LastInputType = ECommonInputType::MouseAndKeyboard;
  float HoldProgress = 0.f;
  FDelegateHandle ThemeHandle, InputHandle;
};
