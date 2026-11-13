#pragma once
#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "DrScrollBox.generated.h"

class SDrScrollEdge;

/**
 * 스크롤 영역. 스크롤·관성·패드 이동은 엔진 UScrollBox가 맡고, 여기서는 얇은 알약 막대(트랙 없음)를 토큰으로 칠하고
 * 위·아래(가로면 왼쪽·오른쪽)에 더 볼 내용이 있을 때만 옅은 가장자리 그늘을 보인다.
 */
UCLASS()
class DRUIKIT_API UDrScrollBox : public UScrollBox {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ThumbToken = TEXT("ink-faint");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ThumbHoverToken = TEXT("ink-muted");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float BarThickness = 4.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bEdgeShadows = true;
  /** 가장자리 그늘 색(이 토큰을 옅게). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName EdgeToken = TEXT("ink");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float EdgeSize = 14.f;
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrScrollEdge> StartEdge, EndEdge;
  FDelegateHandle ThemeHandle;
};
