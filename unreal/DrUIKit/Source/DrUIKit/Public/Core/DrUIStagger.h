#pragma once
#include "CoreMinimal.h"

class UPanelWidget;

/**
 * 순차 등장(디자인 시스템 '움직임': 목록·카드가 하나씩 살짝 올라오며 나타남). 패널의 보이는 자식들에 차례로 지연을 주고
 * 불투명도 0→1, 아래 Rise px → 제자리를 이즈 아웃으로 움직인다. 위젯이 아니라 아무 패널(세로 상자·줄 상자 등)에 건다.
 * 움직임 줄이기면 바로 끝 상태. 같은 패널에 다시 부르면 처음부터 다시 한다.
 */
namespace DrUIMotion {
/** Index번째 항목의 진행(0~1, 이즈 아웃 적용). Elapsed는 시작 뒤 흐른 초. */
DRUIKIT_API float StaggerProgress(float Elapsed, int32 Index, float Step = .04f, float Duration = .28f, int32 MaxItems = 8);
/** 패널 자식들을 순차 등장시킨다. */
DRUIKIT_API void PlayStagger(UPanelWidget* Panel, float Step = .04f, float Rise = 10.f, float Duration = .28f);
}
