#pragma once
#include "CoreMinimal.h"
#include "Components/Border.h"
#include "DrUISurface.generated.h"

class SBackgroundBlur;

/**
 * 표면(패널·카드 공통). 슬롯: 뒤 흐림, 채움 + 테두리(둥근 모서리), 내용.
 * 코어는 토큰 값대로 칠하기만 한다. 유리처럼 보이는지는 테마(샘플)가 정한다.
 * 표면 품질이 낮음이면 흐림을 끄고 FillStrongToken으로 칠한다(글자 대비 유지).
 */
UCLASS()
class DRUIKIT_API UDrUISurface : public UBorder {
  GENERATED_BODY()
public:
  UDrUISurface();
  /** 비우면 투명(테두리만). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("glass");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillStrongToken = TEXT("glass-strong");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName StrokeToken = TEXT("glass-stroke");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName RadiusToken = TEXT("radius-lg");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName PaddingToken = TEXT("space-4");
  /** 비우면 흐림 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName BackdropToken = TEXT("backdrop-glass");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float StrokeWidth = 1.f;

  /** 지금 칠할 채움색(품질 단계 반영). 테스트용으로 공개. */
  FLinearColor ResolveFill() const;
  float ResolveBlur() const;
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SBackgroundBlur> Blur;
  FDelegateHandle ThemeHandle;
};
