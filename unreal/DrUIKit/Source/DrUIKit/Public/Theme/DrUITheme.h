#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/CompositeFont.h"
#include "Fonts/SlateFontInfo.h"
#include "DrUITheme.generated.h"

/** 글자 스타일 토큰 하나(디자인 시스템 type.styles). Size는 언리얼 글꼴 크기(포인트). */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrUITextStyle {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float Size = 14.f;
  /** CSS 굵기(400 보통, 600 반굵게, 700 굵게). 글꼴의 서체 이름으로 바꿔 고른다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") int32 Weight = 400;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float LetterSpacing = 0.f;
  /** 글꼴 에셋(UFont). 비우면 테마의 RuntimeFont, 그것도 없으면 엔진 기본 글꼴. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TSoftObjectPtr<const UObject> Font;
  /** 거짓이면 테마의 RuntimeFont를 쓰지 않는다(코드 글꼴처럼 엔진 기본을 쓸 스타일). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") bool bUseThemeFont = true;
};

/** 그림자 토큰(CSS box-shadow 한 겹). 그리는 방법은 표면 스타일이 정한다. */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrUIShadow {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FVector2D Offset = FVector2D::ZeroVector;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float Blur = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float Spread = 0.f;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FLinearColor Color = FLinearColor::Transparent;
};

/**
 * 테마 하나 = 토큰 묶음. 이름은 디자인 시스템 토큰 이름 그대로(ground, glass, ink, accent, body, radius-lg …).
 * 코어에는 값이 없다: 회색조 기본 테마(MakeDefault)만 코드로 만들고, 실제 테마는 샘플이 tokens.json에서 가져온다.
 */
UCLASS(BlueprintType)
class DRUIKIT_API UDrUITheme : public UPrimaryDataAsset {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FName ThemeId = TEXT("default");
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FLinearColor> Colors;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FDrUITextStyle> TextStyles;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, float> Spacing;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, float> Radius;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FDrUIShadow> Shadows;
  /** 뒤 흐림 강도(UBackgroundBlur BlurStrength). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, float> Backdrops;
  /** 표면 투명도 설정을 따르는 색 토큰(유리 채움처럼 반투명이 뜻인 것). 글자·강조색은 넣지 않는다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TSet<FName> TranslucentTokens;

  /** 코드로 만든 복합 글꼴(TTF 파일 경로 기반, 에셋 없음). 샘플이 넣는다. */
  TSharedPtr<const FCompositeFont> RuntimeFont;

  /** 없는 토큰은 눈에 띄는 자홍색 + 경고 한 번. */
  FLinearColor GetColor(FName Token) const;
  FDrUITextStyle GetTextStyle(FName Token) const;
  float GetSpacing(FName Token) const;
  float GetRadius(FName Token) const;
  FDrUIShadow GetShadow(FName Token) const;
  float GetBackdrop(FName Token) const;
  FSlateFontInfo MakeFont(FName TextToken) const;
  /** 투명도 설정을 반영한 알파. 0 → 1(불투명), .5 → Alpha 그대로, 1 → Alpha의 1/4. */
  static float AdjustAlpha(float Alpha, float Transparency);
  /** 복합 글꼴에서 CSS 굵기에 가장 가까운 서체 이름(Light·Regular·Medium·SemiBold·Bold 중 있는 것). */
  static FName PickTypeface(const FCompositeFont& Font, int32 Weight);

  /** 샘플 없이도 화면이 깨지지 않는 회색조 기본 테마. */
  static UDrUITheme* MakeDefault(UObject* Outer);
  static const FLinearColor MissingColor;
private:
  void WarnMissing(const TCHAR* Kind, FName Token) const;
  mutable TSet<FName> Warned;
};

/** 코어 부품이 쓰는 토큰 이름(디자인 시스템과 같은 이름). */
namespace DrUITokens {
inline const FName Ground = TEXT("ground"), Glass = TEXT("glass"), GlassStrong = TEXT("glass-strong"), GlassStroke = TEXT("glass-stroke");
inline const FName Hairline = TEXT("hairline"), Ink = TEXT("ink"), InkMuted = TEXT("ink-muted"), InkFaint = TEXT("ink-faint");
inline const FName Accent = TEXT("accent"), OnAccent = TEXT("on-accent"), AccentSoft = TEXT("accent-soft"), Chip = TEXT("chip"), OnChip = TEXT("on-chip");
inline const FName Success = TEXT("success"), Warning = TEXT("warning"), Danger = TEXT("danger"), Focus = TEXT("focus");
inline const FName Body = TEXT("body"), Label = TEXT("label"), Caption = TEXT("caption");
inline const FName RadiusXs = TEXT("radius-xs"), RadiusSm = TEXT("radius-sm"), RadiusMd = TEXT("radius-md"), RadiusLg = TEXT("radius-lg"), RadiusXl = TEXT("radius-xl"), RadiusPill = TEXT("radius-pill");
inline const FName Space2 = TEXT("space-2"), Space3 = TEXT("space-3"), Space4 = TEXT("space-4");
inline const FName BackdropGlass = TEXT("backdrop-glass"), BackdropSoft = TEXT("backdrop-soft"), ShadowGlass = TEXT("shadow-glass"), ShadowFloat = TEXT("shadow-float");
}
