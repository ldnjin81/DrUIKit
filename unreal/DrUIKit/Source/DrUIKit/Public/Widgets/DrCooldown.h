#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "DrCooldown.generated.h"

class SDrCooldown;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDrCooldownEvent);

/**
 * 쿨다운 표시. 아이콘 위에 겹쳐 두면 남은 비율만큼 어두운 막이 12시부터 시계 방향으로 걷히고, 가운데에 남은 시간이 보인다.
 * 다 끝나면 테두리가 한 번 반짝인다(움직임 줄이기면 반짝임 없음). 시간은 Slate 시계로 흐르므로 틱 없이도 맞다.
 */
UCLASS()
class DRUIKIT_API UDrCooldown : public UWidget {
  GENERATED_BODY()
public:
  /** 어두운 막 색(scrim). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ScrimToken = TEXT("scrim");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName TextToken = TEXT("on-accent");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FlashToken = TEXT("accent");
  /** 막 모서리 반경 토큰(아이콘 칩과 맞춤). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName RadiusToken = TEXT("radius-md");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowSeconds = true;

  UFUNCTION(BlueprintCallable, Category = "DrUI") void StartCooldown(float Seconds);
  /** 진행 중인 쿨다운을 지정 상태로(남은 초 / 전체 초). Remaining이 0 이하면 끝난 상태. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetRemaining(float Remaining, float Total);
  UFUNCTION(BlueprintCallable, Category = "DrUI") bool IsCoolingDown() const;
  UFUNCTION(BlueprintCallable, Category = "DrUI") float GetRemaining() const;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrCooldownEvent OnFinished;

  /** 남은 비율(0~1). */
  static float Fraction(float Remaining, float Total) { return Total > 0.f ? FMath::Clamp(Remaining / Total, 0.f, 1.f) : 0.f; }
  /** 남은 시간 글: 10초 이상은 올림 정수("12"), 미만은 소수 한 자리("3.2"), 끝나면 빈 글. */
  static FText FormatRemaining(float Remaining);
  /** 둥근 사각형(크기 Size, 모서리 Radius) 안에서 남은 Fraction만큼(12시까지 시계 방향 끝부분) 덮는 부채꼴 꼭짓점(가운데 + 테두리 위 점들). */
  static TArray<FVector2f> SweepPolygon(FVector2f Size, float Fraction, float Radius = 0.f);
  void ApplyTheme();
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void SynchronizeProperties() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  double EndTime = 0.0;
  float Total = 0.f;
  TSharedPtr<SDrCooldown> Slate;
  FDelegateHandle ThemeHandle;
};
