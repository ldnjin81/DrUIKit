#pragma once
#include "CoreMinimal.h"
#include "Widgets/DrButton.h"
#include "DrHoldButton.generated.h"

class USizeBox;

/**
 * 홀드 버튼(길게 눌러야 실행: 삭제·구매·건너뛰기처럼 실수하면 안 되는 행동). 누르고 있는 동안 바탕이 왼쪽부터 차오르고, 다 차면 클릭된다.
 * 누르기·진행·되감기·클릭 판정은 CommonUI 버튼의 길게 누르기(bRequiresHold)를 그대로 쓰고, 시간만 여기 값으로 덮어쓴다.
 * 중간에 손을 떼면 RollbackSeconds 동안 되감긴다. 채움 색은 글자색을 옅게(어느 변형 위에서도 보이게).
 */
UCLASS()
class DRUIKIT_API UDrHoldButton : public UDrButton {
  GENERATED_BODY()
public:
  UDrHoldButton(const FObjectInitializer& Initializer);
  /** 다 차는 데 걸리는 시간(초). 키보드·마우스·패드·터치 모두 같은 값. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0.05)) float HoldSeconds = 1.f;
  /** 손을 뗐을 때 다 찬 상태에서 0까지 되감기는 시간(초). 0이면 바로 0. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0)) float RollbackSeconds = .35f;
  /** 채움 불투명도(글자색 위에 곱함). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 1)) float FillOpacity = .22f;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetHoldSeconds(float Seconds);
  /** 지금 진행(0~1). */
  float GetHoldProgress() const { return CurrentHoldProgress; }
  virtual void UpdateHoldData(ECommonInputType CurrentInputType) override;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<USizeBox> FillBox;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Fill;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void ApplyHoldTimes();
};
