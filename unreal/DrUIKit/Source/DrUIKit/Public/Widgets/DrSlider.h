#pragma once
#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "DrSlider.generated.h"

class SDrSliderTrack;

/**
 * 슬라이더. 끌기·키보드·패드 조작은 엔진 USlider가 맡고, 여기서는 둥근 트랙과 채워진 부분(값까지 accent),
 * 테두리 있는 손잡이를 토큰으로 칠한다. 엔진 슬라이더 막대는 그리지 않고 그 뒤에 트랙을 직접 그린다.
 * 단계가 MaxTicks 이하이면 눈금 점을 찍고, 끄는 동안·키보드나 패드로 바꾼 직후에는 손잡이 위에 값 말풍선을 띄운다.
 */
UCLASS()
class DRUIKIT_API UDrSlider : public USlider {
  GENERATED_BODY()
public:
  UDrSlider();
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TrackToken = TEXT("hairline");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ThumbToken = TEXT("on-accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float TrackThickness = 4.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float ThumbSize = 18.f;
  /** 눈금 점. 단계 수가 MaxTicks보다 많으면(촘촘하면) 그리지 않는다. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowTicks = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 2)) int32 MaxTicks = 12;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowValueBubble = true;

  /** 값의 비율(0~1). 최솟값·최댓값을 반영. */
  float GetFraction() const;
  /** 채워진 부분의 오른쪽 끝(px). 손잡이 가운데와 같다(손잡이는 양 끝에서 반 크기만큼 안쪽까지 움직인다). */
  static float FillEnd(float Width, float Fraction, float InThumbSize) { return InThumbSize * .5f + FMath::Clamp(Fraction, 0.f, 1.f) * FMath::Max(0.f, Width - InThumbSize); }
  /** 눈금 칸 수(점은 이것 + 1개). 단계가 없거나 MaxSteps보다 많으면 0. */
  static int32 TickSteps(float Min, float Max, float Step, int32 MaxSteps);
  /** 말풍선 글. 0~1 범위면 백분율, 아니면 단계에 맞춘 소수 자리. 덮어써서 단위를 붙일 수 있다. */
  virtual FText GetBubbleText() const;
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  UFUNCTION() void HandleCaptureBegin();
  UFUNCTION() void HandleCaptureEnd();
  UFUNCTION() void HandleValueChangedForBubble(float InValue);
  bool IsBubbleVisible() const;
  bool bCaptured = false;
  double BubbleUntil = 0.0;
  TSharedPtr<SDrSliderTrack> Track;
  FDelegateHandle ThemeHandle;
};
