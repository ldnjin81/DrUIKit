#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Containers/Ticker.h"
#include "Core/DrUIMotion.h"
#include "DrConfirmDialog.generated.h"

class UBorder;
class UDrButton;
class UDrSurface;
class UDrText;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrConfirmResult, bool, bConfirmed);

/**
 * 확인 대화상자(모달). 뒤를 scrim으로 막고 가운데 유리 판에 제목·설명·취소·확인. 파괴적 행동(삭제 등)이면 확인 버튼이 위험 색이고
 * 첫 포커스가 취소에 간다(실수로 확정하지 않게). 뒤로(Esc·패드 B)는 취소. 결과를 알리고 스스로 닫힌다.
 * 보통 UDrUIFlowSubsystem::ShowConfirm으로 모달 층에 띄운다.
 */
UCLASS()
class DRUIKIT_API UDrConfirmDialog : public UCommonActivatableWidget {
  GENERATED_BODY()
public:
  UDrConfirmDialog(const FObjectInitializer& Initializer);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetContent(const FText& InTitle, const FText& InBody, const FText& InConfirmLabel, const FText& InCancelLabel, bool bInDestructive);
  /** 결과를 알리고 닫는다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Resolve(bool bConfirmed);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrConfirmResult OnResult;
  /** C++에서 받을 때(한 번만 불린다). */
  TFunction<void(bool)> OnResultNative;
  virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UBorder> Scrim;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Panel;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> TitleText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> BodyText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> ConfirmButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButton> CancelButton;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnActivated() override;
  virtual UWidget* NativeGetDesiredFocusTarget() const override;
  virtual bool NativeOnHandleBackAction() override;
  virtual void NativeOnDeactivated() override;
private:
  /** 활성 위젯은 NativeTick이 꺼져 있을 수 있어(DisableNativeTick) 코어 티커로 움직인다. */
  bool StepPresence(float DeltaTime);
  FTSTicker::FDelegateHandle Ticker;
  void ApplyContent();
  void ApplyPresence();
  FText Title, Body, ConfirmLabel, CancelLabel;
  bool bDestructive = false, bResolved = false;
  FDrUISpring Presence;
};
