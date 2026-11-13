#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "DrPageIndicator.generated.h"

class SDrPageIndicator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrPageEvent, int32, Page);

namespace DrPageIndicatorLayout {
/** Index번째 점의 너비. 지금 페이지(Active, 스프링이라 소수)에 가까울수록 Wide로 늘어난다. */
DRUIKIT_API float DotWidth(int32 Index, float Active, float Dot, float Wide);
/** 전체 너비(점 Count개, 하나는 늘어난 상태). */
DRUIKIT_API float TotalWidth(int32 Count, float Dot, float Wide, float Gap);
}

/**
 * 페이지 인디케이터(점). 지금 페이지 점이 알약으로 늘어나고, 페이지가 바뀌면 스프링으로 옆 점으로 옮겨 간다.
 * 점을 누르면 OnPageClicked(번호) — 실제 페이지 전환은 쓰는 쪽이 하고 SetCurrent로 알려 준다.
 */
UCLASS()
class DRUIKIT_API UDrPageIndicator : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0)) int32 PageCount = 3;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Current = 0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float DotSize = 8.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float ActiveWidth = 22.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Gap = 8.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName DotToken = TEXT("ink-faint");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ActiveToken = TEXT("ink");

  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetPageCount(int32 InCount);
  /** 지금 페이지를 바꾼다(움직임 줄이기면 바로). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetCurrent(int32 Page);

  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrPageEvent OnPageClicked;
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrPageIndicator> Slate;
  FDelegateHandle ThemeHandle;
};
