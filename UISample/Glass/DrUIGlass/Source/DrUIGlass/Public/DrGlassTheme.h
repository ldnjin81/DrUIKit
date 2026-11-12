#pragma once
#include "CoreMinimal.h"

class UDrUITheme;

/** 디자인 시스템 tokens.json(Dr UI 디자인 언어)을 읽어 테마(테마당 하나: glass-light, glass-dark)를 만든다. */
namespace DrGlassTheme {
/** CSS 색(#rgb, #rrggbb, #rrggbbaa, rgb(), rgba())을 sRGB로 읽는다. 실패하면 false. */
DRUIGLASS_API bool ParseCssColor(const FString& Text, FLinearColor& Out);
/** tokens.json 글자 크기(px)를 언리얼 글꼴 크기로. 언리얼 글꼴 크기 1 = 96dpi에서 4/3px. */
DRUIGLASS_API float PxToFontSize(float Px);
/** JSON 문자열에서 테마들을 만든다. 테마 id 앞에 "glass-"를 붙인다. */
DRUIGLASS_API TArray<UDrUITheme*> BuildThemes(const FString& Json, UObject* Outer, FString& OutError);
/** 플러그인 Tokens/tokens.json을 읽어 만든다. */
DRUIGLASS_API TArray<UDrUITheme*> LoadThemes(UObject* Outer, FString& OutError);
}
