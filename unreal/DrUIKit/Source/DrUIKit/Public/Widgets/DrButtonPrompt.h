#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonInputTypeEnum.h"
#include "Core/DrUIInput.h"
#include "InputCoreTypes.h"
#include "DrButtonPrompt.generated.h"

class UDrKeyGlyph;
class UDrProgressBar;
class UDrText;

UENUM(BlueprintType)
enum class EDrPromptDevice : uint8 {
  /** 지금 쓰는 입력 장치를 따라 바뀐다(CommonInput). */
  Auto,
  Keyboard,
  Gamepad,
  /** 패드 키와 키보드 키를 함께("Ⓐ / Enter"). 둘 중 하나만 정했으면 그것만. */
  Both
};

/**
 * 버튼 프롬프트("Ⓐ 확인", "Esc 뒤로"). 키를 실제 모양(UDrKeyGlyph: 패드는 Xbox·PlayStation·Nintendo 버튼 모양, 키보드는 키캡,
 * 마우스는 마우스)으로 그리고 옆에 행동 이름을 붙인다. 키보드·패드 키를 따로 정해 두면 마지막으로 쓴 입력 장치에 맞춰 바뀌고,
 * Both면 둘 다 보인다. 아이콘 그림 에셋 없이 그린다(엔진 UCommonActionWidget은 컨트롤러 데이터 에셋의 아이콘이 있어야 보인다).
 * SetHoldProgress로 길게 누르기 진행을 키 아래 얇은 막대로 보인다.
 */
UCLASS()
class DRUIKIT_API UDrButtonPrompt : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey KeyboardKey = EKeys::Enter;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FKey GamepadKey = EKeys::Gamepad_FaceButton_Bottom;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrPromptDevice Device = EDrPromptDevice::Auto;
  /** 패드 표기. 자동이면 설정·지금 패드 종류. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGamepadGlyphs Glyphs = EDrGamepadGlyphs::Auto;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float KeyHeight = 24.f;
  /** 얼굴 버튼을 패드 고유색으로. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bBrandColors = false;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetKeys(FKey InKeyboardKey, FKey InGamepadKey, const FText& InLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetDevice(EDrPromptDevice InDevice);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetGlyphs(EDrGamepadGlyphs InGlyphs);
  /** 0이면 막대 숨김. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetHoldProgress(float Progress);

  /** 키 이름(Xbox 표기: A·B·X·Y·LB·RT·LS·←, 키보드 Esc·Enter·Space·Tab·F1 등). 다른 표기는 DrUIInput::GlyphName. */
  static FText ShortKeyName(const FKey& Key);
  /** 원 모양으로 그리는 키(패드 얼굴 버튼·스틱). */
  static bool IsRoundKey(const FKey& Key);
  /** 지금 보일 키(Both면 패드 키). */
  FKey GetShownKey() const;
  /** 지금 패드 키·키보드 키가 각각 보이는지. */
  void GetShownKeys(bool& bOutGamepad, bool& bOutKeyboard) const;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrKeyGlyph> GamepadGlyph;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> Separator;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrKeyGlyph> KeyboardGlyph;
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
