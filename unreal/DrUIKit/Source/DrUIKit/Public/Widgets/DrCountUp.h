#pragma once
#include "CoreMinimal.h"
#include "CommonNumericTextBlock.h"
#include "DrCountUp.generated.h"

/**
 * 카운트 업 숫자(통계 숫자가 새 값까지 세어 올라감). 숫자 형식(정수·백분율·초·거리)과 보간은 CommonUI 숫자 글(UCommonNumericTextBlock)이 맡고,
 * 여기서는 글자 스타일·색 토큰과 CountTo(움직임 줄이기면 바로 그 값)를 더한다.
 */
UCLASS()
class DRUIKIT_API UDrCountUp : public UCommonNumericTextBlock {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TextStyle = TEXT("metric");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink");
  /** 새 값까지 걸리는 최대 시간(초). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float CountSeconds = .8f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void CountTo(float Target);
  void SetTokens(FName InTextStyle, FName InColorToken);
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  FDelegateHandle ThemeHandle;
};
