#pragma once
#include "CoreMinimal.h"
#include "Core/DrUIInput.h"
#include "InputCoreTypes.h"

class UTexture2D;

/** 불러온 버튼 이미지 세트 하나(플러그인 하나). */
struct DRUIKIT_API FDrUIGlyphImageSetInfo {
  FName Id;
  FText Name;
  FString Plugin;
  /** 작을수록 우선(같은 키를 여러 세트가 가지면 order가 작은 세트). */
  int32 Order = 100;
  /** 이미지 라이선스(표시용). */
  FString License;
  /** 흰색(단색) 이미지에 테마 색을 입힐지(패드 키는 chip, 키보드·마우스는 ink). 색이 든 공식 이미지는 false. */
  bool bTint = true;
  int32 ImageCount = 0;
};

/**
 * 버튼 이미지 세트. 키 모양(UDrKeyGlyph)은 이미지가 있으면 이미지를, 없으면 벡터 그림을 쓴다.
 * 플러그인의 Glyphs/glyphs.json:
 *   { "id": "kenney", "name": "Kenney Input Prompts", "order": 50, "license": "CC0 1.0", "tint": true,
 *     "sets": { "xbox": { "Gamepad_FaceButton_Bottom": "Xbox/xbox_button_a.png", … },
 *               "playstation": { … }, "nintendo": { … }, "keyboard": { "Escape": "Keyboard/keyboard_escape.png", "LeftMouseButton": … },
               "xbox_brand": { "$tint": false, "Gamepad_FaceButton_Bottom": "Xbox/xbox_button_color_a.png", … } } }
 * 키 이름은 EKeys 이름(FKey::GetFName). 경로는 Glyphs 폴더 기준. 키보드·마우스는 "keyboard" 세트에서 찾는다.
 * "<표기>_brand" 세트는 고유색(bBrandColors) 그림, 세트 안 "$tint"는 그 세트만 색 입히기를 바꾼다.
 * PNG는 처음 쓸 때 실행 중에 텍스처로 만든다(에셋 없음). 실제 출시 게임은 플랫폼 포털에서 받은 공식 이미지로 세트를 만들어
 * 비공개 플러그인으로 꽂으면 된다(order를 더 작게). 패키지에 넣으려면 Glyphs 폴더를 RuntimeDependencies로 함께 넣는다.
 */
namespace DrUIGlyphImages {
/** JSON 문자열로 세트를 등록한다(경로는 BaseDir 기준). 실패하면 false와 OutError. */
DRUIKIT_API bool Register(const FString& Json, const FString& BaseDir, const FString& PluginName, FDrUIGlyphImageSetInfo& OutInfo, FString& OutError);
/** 켜진 플러그인 중 Glyphs/glyphs.json이 있는 것을 모두 등록한다(코어 모듈이 엔진 시작 때 부른다). */
DRUIKIT_API void LoadAll();
DRUIKIT_API void Clear();
DRUIKIT_API const TArray<FDrUIGlyphImageSetInfo>& GetLoadedSets();
/** 표기(Xbox·PlayStation·Nintendo)와 키로 이미지. 키보드·마우스 키는 표기와 무관하게 keyboard 세트. 없거나 이미지 쓰기가 꺼져 있으면 nullptr. */
/** bBrand면 "<표기>_brand" 세트를 먼저 찾는다. */
DRUIKIT_API UTexture2D* Find(EDrUIGamepadGlyphs Glyphs, const FKey& Key, bool* bOutTint = nullptr, bool bBrand = false);
/** 세트 이름(JSON의 sets 키)과 키로 등록된 이미지 경로(테스트·디버그용, 없으면 빈 문자열). */
DRUIKIT_API FString FindPath(FName SetName, const FKey& Key);
}
