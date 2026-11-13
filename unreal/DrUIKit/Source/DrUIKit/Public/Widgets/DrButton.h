#pragma once
#include "CoreMinimal.h"
#include "Widgets/DrButtonBase.h"
#include "DrButton.generated.h"

class UDrSurface;
class UDrText;
class UDrSpinner;

UENUM(BlueprintType)
enum class EDrButtonVariant : uint8 { Primary, Secondary, Ghost, Destructive };

/**
 * 버튼(디자인 시스템 '버튼 상태 9가지'). 공통 상태 판정은 UDrButtonBase, 여기서는 로딩·성공·오류와 4종 칠하기를 더한다.
 * 하위 부품은 BindWidgetOptional: 위젯 블루프린트가 같은 이름으로 두면 그걸 쓰고, 없으면 C++가 기본 구성을 만든다.
 */
UCLASS()
class DRUIKIT_API UDrButton : public UDrButtonBase {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrButtonVariant Variant = EDrButtonVariant::Primary;
  /** 성공 상태가 기본으로 돌아가기까지(초). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float SuccessHoldSeconds = 1.5f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLabel(const FText& InLabel);
  /** 로딩 중에는 눌러도 반응하지 않는다(크기 유지, 스피너). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLoading(bool bInLoading);
  /** 성공: SuccessHoldSeconds 뒤 기본으로. SuccessLabel이 비면 Label 유지. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void ShowSuccess(const FText& SuccessLabel);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetError(bool bInError, const FText& ErrorLabel);

  virtual FDrUIStateInputs GetStateInputs() const override;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSpinner> Spinner;

  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
  bool bLoading = false, bError = false;
  float SuccessUntil = -1.f;
  FText ShownLabel;
};
