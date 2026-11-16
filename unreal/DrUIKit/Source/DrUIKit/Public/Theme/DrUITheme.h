#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/CompositeFont.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "Styling/SlateBrush.h"
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
  /** 거짓이면 테마의 RuntimeFonts를 쓰지 않는다(엔진 기본을 쓸 스타일). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") bool bUseThemeFont = true;
  /** 글꼴 갈래(tokens.json type.groups의 family: sans, mono). 테마에 그 갈래 글꼴이 없으면 sans는 RuntimeFont, 그 밖은 엔진 기본. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FName Family = TEXT("sans");
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

/** 그림자 토큰 하나의 모든 겹(CSS box-shadow의 쉼표로 나뉜 바깥 겹들. 뉴모피즘의 밝은 겹 + 어두운 겹 등). */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrUIShadowLayers {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TArray<FDrUIShadow> Layers;
};

/** 이미지 토큰 맞춤 방법. */
UENUM(BlueprintType)
enum class EDrUIImageFit : uint8 {
  /** 9분할: 여백(모서리·테두리)은 그대로, 가운데만 늘린다(패널 테두리). */
  Box,
  /** 비율을 지키며 칸을 가득 채우고 넘치는 쪽은 자른다(화면 바탕). */
  Cover
};

/** 이미지 토큰 하나(style.json images). 색 토큰 대신 그림으로 칠한다. 파일은 처음 쓸 때 텍스처로 만든다(에셋 없음). */
USTRUCT(BlueprintType)
struct DRUIKIT_API FDrUIImage {
  GENERATED_BODY()
  /** PNG·JPG 절대 경로. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FString Path;
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") EDrUIImageFit Fit = EDrUIImageFit::Box;
  /** 9분할 여백(이미지 픽셀). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FMargin Margin;
  /** 화면에 그릴 배율(2배 해상도로 그린 이미지는 .5). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float Scale = 1.f;
  /** 곱할 색(바탕을 어둡게 누르는 등). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FLinearColor Tint = FLinearColor::White;
};

/**
 * 테마 하나 = 토큰 묶음. 이름은 디자인 시스템 토큰 이름 그대로(ground, glass, ink, accent, body, radius-lg …).
 * 코어에는 값이 없다: 회색조 기본 테마(MakeDefault)만 코드로 만들고, 실제 테마는 스타일 플러그인의 tokens.json에서 가져온다(DrUIStyle).
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
  /** 그림자 토큰의 첫 바깥 겹. 모든 겹은 ShadowLayers. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FDrUIShadow> Shadows;
  /** 그림자 토큰의 모든 바깥 겹. 없는 토큰은 Shadows의 한 겹을 쓴다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FDrUIShadowLayers> ShadowLayers;
  /** 뒤 흐림 강도(UBackgroundBlur BlurStrength). */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, float> Backdrops;
  /** 표면 투명도 설정을 따르는 색 토큰(유리 채움처럼 반투명이 뜻인 것). 글자·강조색은 넣지 않는다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TSet<FName> TranslucentTokens;
  /** 표면 테두리의 기본 두께(브루탈리즘처럼 굵은 테두리). 부품의 1px 대신 이 값, 강조 두께(2px 등)는 그 차이만큼 더한다. 0이면 부품 값 그대로. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") float StrokeWidth = 0.f;
  /** 색 토큰 대신 그림으로 칠할 토큰(ground → 화면 바탕, glass → 패널 9분할 등). 스타일 로더가 style.json images에서 넣는다. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") TMap<FName, FDrUIImage> Images;
  /** 이 테마를 만든 스타일(glass, flat …). 코어 기본 테마는 None. */
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DrUI") FName StyleId;

  /** 본문(sans) 복합 글꼴(TTF 파일 경로 기반, 에셋 없음). 스타일 로더가 넣는다. */
  TSharedPtr<const FCompositeFont> RuntimeFont;
  /** 글꼴 갈래별 복합 글꼴(sans, mono …). sans는 RuntimeFont와 같다. */
  TMap<FName, TSharedPtr<const FCompositeFont>> RuntimeFonts;

  /** 없는 토큰은 눈에 띄는 자홍색 + 경고 한 번. */
  FLinearColor GetColor(FName Token) const;
  FDrUITextStyle GetTextStyle(FName Token) const;
  float GetSpacing(FName Token) const;
  float GetRadius(FName Token) const;
  FDrUIShadow GetShadow(FName Token) const;
  /** 그림자 토큰의 모든 바깥 겹(없는 토큰이면 빈 배열 + 경고). */
  TArray<FDrUIShadow> GetShadowLayers(FName Token) const;
  float GetBackdrop(FName Token) const;
  /** 토큰에 이미지가 있으면 그 브러시(텍스처를 처음 쓸 때 만든다). 없거나 파일을 읽지 못하면 false(그때는 색으로 칠한다). 경고 없음. */
  bool MakeImageBrush(FName Token, FSlateBrush& Out) const;
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
inline const FName Ground = TEXT("ground"), Glass = TEXT("glass"), GlassStrong = TEXT("glass-strong"), GlassStroke = TEXT("glass-stroke"), GlassHighlight = TEXT("glass-highlight");
inline const FName Hairline = TEXT("hairline"), Ink = TEXT("ink"), InkMuted = TEXT("ink-muted"), InkFaint = TEXT("ink-faint");
inline const FName Accent = TEXT("accent"), OnAccent = TEXT("on-accent"), AccentSoft = TEXT("accent-soft"), Chip = TEXT("chip"), OnChip = TEXT("on-chip");
inline const FName Success = TEXT("success"), Warning = TEXT("warning"), Danger = TEXT("danger"), Focus = TEXT("focus");
inline const FName Body = TEXT("body"), Label = TEXT("label"), Caption = TEXT("caption");
inline const FName RadiusXs = TEXT("radius-xs"), RadiusSm = TEXT("radius-sm"), RadiusMd = TEXT("radius-md"), RadiusLg = TEXT("radius-lg"), RadiusXl = TEXT("radius-xl"), RadiusPill = TEXT("radius-pill");
inline const FName Space2 = TEXT("space-2"), Space3 = TEXT("space-3"), Space4 = TEXT("space-4");
inline const FName BackdropGlass = TEXT("backdrop-glass"), BackdropSoft = TEXT("backdrop-soft"), ShadowGlass = TEXT("shadow-glass"), ShadowFloat = TEXT("shadow-float");
}
