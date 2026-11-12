#pragma once
#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "DrUISlider.generated.h"

class SDrUISliderTrack;

/**
 * 슬라이더. 끌기·키보드·패드 조작은 엔진 USlider가 맡고, 여기서는 둥근 트랙과 채워진 부분(값까지 accent),
 * 테두리 있는 손잡이를 토큰으로 칠한다. 엔진 슬라이더 막대는 그리지 않고 그 뒤에 트랙을 직접 그린다.
 */
UCLASS()
class DRUIKIT_API UDrUISlider : public USlider {
  GENERATED_BODY()
public:
  UDrUISlider();
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TrackToken = TEXT("hairline");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ThumbToken = TEXT("on-accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float TrackThickness = 4.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float ThumbSize = 18.f;

  /** 값의 비율(0~1). 최솟값·최댓값을 반영. */
  float GetFraction() const;
  /** 채워진 부분의 오른쪽 끝(px). 손잡이 가운데와 같다(손잡이는 양 끝에서 반 크기만큼 안쪽까지 움직인다). */
  static float FillEnd(float Width, float Fraction, float InThumbSize) { return InThumbSize * .5f + FMath::Clamp(Fraction, 0.f, 1.f) * FMath::Max(0.f, Width - InThumbSize); }
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrUISliderTrack> Track;
  FDelegateHandle ThemeHandle;
};
