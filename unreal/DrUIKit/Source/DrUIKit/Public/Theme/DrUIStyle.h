#pragma once
#include "CoreMinimal.h"

class FJsonObject;
class UDrUITheme;
class UDrUIThemeSubsystem;
struct FDrUIShadow;

/** 불러온 스타일 하나(스타일 플러그인 하나). */
struct DRUIKIT_API FDrUIStyleInfo {
  /** 테마 id 앞부분(glass → glass-light, glass-dark). */
  FName Id;
  FText Name;
  FString Plugin;
  /** 작을수록 앞(갤러리 순서, 처음 테마). */
  int32 Order = 100;
  TArray<FName> ThemeIds;
};

/**
 * 스타일 = 플러그인 하나의 Tokens/tokens.json(디자인 토큰) + Tokens/style.json(이름·글꼴·반투명 토큰·테두리 두께).
 * 켜진 플러그인 중 Tokens/style.json이 있는 것을 스타일로 본다. 테마 id는 "<스타일 id>-<tokens.json 테마 id>".
 *
 * style.json:
 *   { "id": "flat", "name": "플랫", "order": 20, "translucent": ["glass"], "strokeWidth": 2.5,
 *     "fonts": { "sans": { "typefaces": { "Regular": "{engine}/Slate/Fonts/Roboto-Regular.ttf", … },
 *                          "hangul": { "Regular": "{plugin:DrUIGlass}/Fonts/Pretendard-Regular.ttf", … } } } }
 * 이미지(선택): 색 토큰 대신 그림으로 칠할 토큰. {mode}는 tokens.json 테마 id(light·dark). 경로는 플러그인 기준(글꼴처럼 {plugin:이름}·{engine}도 된다).
 *   "images": { "ground": { "file": "Images/ground-{mode}.jpg", "fit": "cover", "tint": "#ffffffd0" },
 *               "glass": { "file": "Images/panel-{mode}.png", "margin": 96, "padding": 40, "scale": 0.5 } }
 *   ground는 화면 바탕(갤러리 바탕 등), 그 밖의 토큰은 그 토큰으로 채우는 표면(UDrSurface)을 9분할로 칠한다(margin은 이미지 픽셀, 숫자 또는 [왼,위,오른,아래]).
 *   padding은 내용 안전 여백(이미지 픽셀, 없으면 margin). 표면은 내용을 적어도 padding × scale만큼 안쪽에 둔다(부품 자체 여백이 더 크면 그대로).
 *   이미지로 칠하는 표면은 뒤 흐림·반사광을 그리지 않는다(그림의 가장자리가 사각형이 아닐 수 있다).
 * 글꼴 경로: {plugin}은 이 스타일 플러그인, {plugin:이름}은 다른 플러그인, {engine}은 엔진 Content 폴더.
 * hangul은 한국어 글자(한글, ₩, CJK 기호·전각 문자)만 대신 그리는 서체(라틴 글꼴에 한글이 없을 때). 서체 이름은 굵기 이름(Light·Regular·Medium·SemiBold·Bold).
 * 패키지에 넣으려면 스타일 파일을 RuntimeDependencies로 함께 넣는다(코드 모듈이 있는 플러그인의 Build.cs 등).
 */
namespace DrUIStyle {
/** CSS 색(#rgb, #rrggbb, #rrggbbaa, rgb(), rgba())을 sRGB로 읽는다. 실패하면 false. */
DRUIKIT_API bool ParseCssColor(const FString& Text, FLinearColor& Out);
/** CSS box-shadow의 바깥 겹들(inset 겹은 건너뛴다. 윗면 반사광 토큰이 맡는다). */
DRUIKIT_API TArray<FDrUIShadow> ParseCssShadows(const FString& Text);
/** 첫 바깥 겹. 없으면 false. */
DRUIKIT_API bool ParseCssShadow(const FString& Text, FDrUIShadow& Out);
/** tokens.json 글자 크기(px)를 언리얼 글꼴 크기로. 언리얼 글꼴 크기 1 = 96dpi에서 4/3px. */
DRUIKIT_API float PxToFontSize(float Px);
/** tokens.json 문자열에서 테마들을 만든다(글꼴·반투명·테두리 두께는 없음). 테마 id = "<StyleId>-<테마 id>". */
DRUIKIT_API TArray<UDrUITheme*> BuildThemes(const FString& TokensJson, FName StyleId, UObject* Outer, FString& OutError);
/** 플러그인의 Tokens/tokens.json + style.json으로 테마를 만든다. 실패 내용은 OutError(글꼴이 없으면 엔진 기본으로 대체하고 알린다). */
DRUIKIT_API TArray<UDrUITheme*> LoadPluginStyle(const FString& PluginName, UObject* Outer, FString& OutError, FDrUIStyleInfo* OutInfo = nullptr);
/** style.json의 images를 테마들에 넣는다(LoadPluginStyle이 부른다). 없는 파일은 OutError에 더하고 건너뛴다. */
DRUIKIT_API void ApplyImages(const TSharedPtr<FJsonObject>& Style, const FString& PluginDir, FName StyleId, const TArray<UDrUITheme*>& Themes, FString& OutError);
/** 켜진 플러그인 중 스타일인 것(order 순). */
DRUIKIT_API TArray<FString> FindStylePlugins();
/** 모든 스타일을 불러 서브시스템에 넣는다(order가 가장 작은 스타일의 첫 테마가 처음 테마). 오류는 로그로. */
DRUIKIT_API void LoadAllStyles(UDrUIThemeSubsystem& Subsystem);
/** LoadAllStyles로 넣은 스타일(order 순). */
DRUIKIT_API const TArray<FDrUIStyleInfo>& GetLoadedStyles();
}
