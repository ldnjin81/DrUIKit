#pragma once
#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Core/DrUIMotion.h"
#include "Core/DrUIState.h"
#include "DrUIButton.generated.h"

class UDrUISurface;
class UDrUIText;
class UCircularThrobber;

UENUM(BlueprintType)
enum class EDrUIButtonVariant : uint8 { Primary, Secondary, Ghost, Destructive };

/** 그리지 않는 버튼 스타일. 모양은 DrUIKit 표면이 칠한다. */
UCLASS()
class DRUIKIT_API UDrUIButtonStyle : public UCommonButtonStyle {
  GENERATED_BODY()
public:
  UDrUIButtonStyle();
};

/**
 * 버튼(디자인 시스템 '버튼 상태 9가지'). CommonUI 버튼을 상속해 입력·포커스·패드·선택은 엔진이,
 * 상태 판정(DrUIState::Resolve)과 칠하기·움직임은 DrUIKit이 맡는다.
 * 하위 부품은 BindWidgetOptional: 위젯 블루프린트가 같은 이름으로 두면 그걸 쓰고, 없으면 C++가 기본 구성을 만든다.
 */
UCLASS()
class DRUIKIT_API UDrUIButton : public UCommonButtonBase {
  GENERATED_BODY()
public:
  UDrUIButton(const FObjectInitializer& Initializer);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrUIButtonVariant Variant = EDrUIButtonVariant::Primary;
  /** 성공 상태가 기본으로 돌아가기까지(초). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float SuccessHoldSeconds = 1.5f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLabel(const FText& InLabel);
  /** 로딩 중에는 눌러도 반응하지 않는다(크기 유지, 스피너). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLoading(bool bInLoading);
  /** 성공: SuccessHoldSeconds 뒤 기본으로. SuccessLabel이 비면 Label 유지. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void ShowSuccess(const FText& SuccessLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetError(bool bInError, const FText& ErrorLabel);

  EDrUIState GetVisualState() const { return VisualState; }
  FDrUIStateInputs GetStateInputs() const;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUISurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUIText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UCircularThrobber> Spinner;

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

  /** 상태가 바뀌면 다시 칠한다. 게임은 이것만 덮어써도 모양을 바꿀 수 있다. */
  virtual void ApplyVisualState();

private:
  void BuildDefaultTree();
  void RefreshState();
  bool bPressedNow = false, bFocusedNow = false, bLoading = false, bError = false;
  float SuccessUntil = -1.f;
  FText ShownLabel;
  EDrUIState VisualState = EDrUIState::Default;
  FDrUISpring Scale;
  FDelegateHandle ThemeHandle;
};
