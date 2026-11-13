#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "DrUIGlyph.generated.h"

class SDrUIGlyph;

/** 코드로 그리는 기본 표시. 체크박스의 체크, 라디오의 점, 선택기의 화살표처럼 부품이 공통으로 쓴다. */
UENUM(BlueprintType)
enum class EDrUIGlyphShape : uint8 { Check, Dot, Minus, Plus, Close, ChevronLeft, ChevronRight, ChevronDown, Alert };

namespace DrUIGlyph {
/** 모양의 획들. 좌표는 0~1 정사각형 안. Dot은 획이 없다(원으로 칠한다). */
DRUIKIT_API TArray<TArray<FVector2f>> Strokes(EDrUIGlyphShape Shape);
/** 꺾은선의 앞에서부터 길이 비율 Fraction(0~1)만큼만 남긴다. 체크를 '그리는' 움직임에 쓴다. */
DRUIKIT_API TArray<FVector2f> Trim(const TArray<FVector2f>& Points, float Fraction);
}

/**
 * 표시 하나(에셋 없이 Slate 선으로 그림). 색은 토큰, 굵기는 크기에 비례.
 * Progress(0~1)는 획을 앞에서부터 그린 정도이고, Dot은 원의 크기 비율이다.
 */
UCLASS()
class DRUIKIT_API UDrUIGlyph : public UWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrUIGlyphShape Shape = EDrUIGlyphShape::Check;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FName ColorToken = TEXT("ink");
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float GlyphSize = 16.f;
  /** 선 굵기(GlyphSize 16 기준 px). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float Thickness = 2.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0, ClampMax = 1)) float Progress = 1.f;

  void SetShape(EDrUIGlyphShape InShape);
  void SetColorToken(FName Token);
  void SetProgress(float InProgress);
  void ApplyTheme();
protected:
  virtual void SynchronizeProperties() override;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  TSharedPtr<SDrUIGlyph> Slate;
  FDelegateHandle ThemeHandle;
};
