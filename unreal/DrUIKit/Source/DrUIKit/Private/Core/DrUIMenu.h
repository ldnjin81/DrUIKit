#pragma once
#include "CoreMinimal.h"
#include "Components/MenuAnchor.h"
#include "UObject/UnrealType.h"

/** 코어 내부: 엔진 메뉴 앵커 설정. */
namespace DrUIMenu {
/** 메뉴 창의 기본 배경(진회색)을 끈다. 엔진 설정 함수(InitShowMenuBackground)가 protected라 속성으로 직접 끈다. Slate를 만들기 전에 부른다. */
inline void HideMenuBackground(UMenuAnchor* Anchor) {
  if (!Anchor) return;
  if (FBoolProperty* Property = FindFProperty<FBoolProperty>(UMenuAnchor::StaticClass(), TEXT("ShowMenuBackground")))
    Property->SetPropertyValue_InContainer(Anchor, false);
}
}
