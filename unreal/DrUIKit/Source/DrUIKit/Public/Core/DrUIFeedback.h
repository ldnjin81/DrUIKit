#pragma once
#include "CoreMinimal.h"
#include "DrUIFeedback.generated.h"

class UWidget;

/** UI 반응 종류. 소리는 설정(UDrUIKitSettings 피드백)에 게임이 넣고, 진동 세기·길이는 종류마다 정해져 있다. */
UENUM(BlueprintType)
enum class EDrUIFeedback : uint8 {
  /** 패드·키보드로 포커스를 옮김(가장 약함). */
  Focus,
  Confirm,
  Back,
  /** 안 되는 동작(두 번 짧게). */
  Error,
  /** 값 한 칸(슬라이더·수평 선택기·수량). */
  Tick
};

namespace DrUIFeedback {
/** 소리(설정에 있고 켜져 있으면)와 진동(패드로 조작 중이고 켜져 있으면). Context는 월드·플레이어를 찾는 데 쓴다. */
DRUIKIT_API void Play(const UWidget* Context, EDrUIFeedback Kind);
/** 종류별 진동(세기 0~1, 길이 초). 테스트·게임이 같은 값을 쓰도록 공개. */
DRUIKIT_API void HapticFor(EDrUIFeedback Kind, float& OutStrength, float& OutSeconds);
}
