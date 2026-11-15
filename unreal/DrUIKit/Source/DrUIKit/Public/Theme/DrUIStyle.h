#pragma once
#include "CoreMinimal.h"

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
/** 켜진 플러그인 중 스타일인 것(order 순). */
DRUIKIT_API TArray<FString> FindStylePlugins();
/** 모든 스타일을 불러 서브시스템에 넣는다(order가 가장 작은 스타일의 첫 테마가 처음 테마). 오류는 로그로. */
DRUIKIT_API void LoadAllStyles(UDrUIThemeSubsystem& Subsystem);
/** LoadAllStyles로 넣은 스타일(order 순). */
DRUIKIT_API const TArray<FDrUIStyleInfo>& GetLoadedStyles();
}
