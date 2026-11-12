#pragma once
#include "CoreMinimal.h"

/** 디자인 시스템 '움직임' 표의 값. 시간은 초. */
namespace DrUIMotion {
/** CSS cubic-bezier(x1,y1,x2,y2)를 t(0~1)에서 푼다. 이즈 아웃 = (.2,.8,.2,1), 이즈 인 = (.4,0,1,1). */
DRUIKIT_API float CubicBezier(float X1, float Y1, float X2, float Y2, float T);
DRUIKIT_API float EaseOut(float T);
DRUIKIT_API float EaseIn(float T);
/** 순차 등장: Index번째 항목의 시작 지연(항목당 Step초, MaxItems개 이후는 같은 지연). */
DRUIKIT_API float StaggerDelay(int32 Index, float Step = 0.03f, int32 MaxItems = 8);
}

/** 감쇠 스프링 하나. 활성 알약이 미끄러지거나 버튼이 눌릴 때 쓴다. */
struct DRUIKIT_API FDrUISpring {
  float Stiffness = 300.f;
  float Damping = 26.f;
  float Value = 0.f;
  float Velocity = 0.f;
  float Target = 0.f;

  /** DeltaTime만큼 진행(큰 프레임은 작은 단계로 나눠 안정). 멈췄으면 true. */
  bool Step(float DeltaTime);
  bool IsSettled(float Tolerance = 1e-3f) const;
  void Snap(float InValue) { Value = Target = InValue; Velocity = 0.f; }
};
