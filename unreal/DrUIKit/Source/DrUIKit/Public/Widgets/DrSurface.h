#pragma once
#include "CoreMinimal.h"
#include "Components/Border.h"
#include "DrSurface.generated.h"

class SBackgroundBlur;
class SDrSurfaceDecor;

/**
 * 표면(패널·카드 공통). 슬롯: 뒤 흐림, 채움 + 테두리(둥근 모서리), 내용.
 * 코어는 토큰 값대로 칠하기만 한다. 유리처럼 보이는지는 테마(샘플)가 정한다.
 * 표면 품질이 낮음이면 흐림을 끄고 FillStrongToken으로 칠한다(글자 대비 유지).
 * 그림자(ShadowToken)와 윗면 반사광(HighlightToken)은 정한 표면만 그린다(기본 끔). 그림자는 표면 바깥에만 그려
 * 반투명 유리 안쪽이 탁해지지 않는다(CSS box-shadow처럼). 흐린 가장자리는 둥근 고리 여러 겹으로 흉내 낸다.
 */
UCLASS()
class DRUIKIT_API UDrSurface : public UBorder {
  GENERATED_BODY()
public:
  UDrSurface();
  /** 비우면 투명(테두리만). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillToken = TEXT("glass");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName FillStrongToken = TEXT("glass-strong");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName StrokeToken = TEXT("glass-stroke");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName RadiusToken = TEXT("radius-lg");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName PaddingToken = TEXT("space-4");
  /** 비우면 흐림 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName BackdropToken = TEXT("backdrop-glass");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float StrokeWidth = 1.f;
  /** 그림자 토큰(shadow-glass·shadow-float). 비우면 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ShadowToken;
  /** 윗변 안쪽 1px 반사광 색 토큰(glass-highlight). 비우면 없음. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName HighlightToken;
  /** 유리 패널·떠 있는 요소 모양을 한 번에(그림자 + 반사광). */
  void SetElevation(FName InShadowToken, FName InHighlightToken = TEXT("glass-highlight")) { ShadowToken = InShadowToken; HighlightToken = InHighlightToken; ApplyTheme(); }

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
  TSharedPtr<SDrSurfaceDecor> Shadow, Highlight;
  FDelegateHandle ThemeHandle;
};
