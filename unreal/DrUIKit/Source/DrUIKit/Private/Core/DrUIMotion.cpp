#include "Core/DrUIMotion.h"

float DrUIMotion::CubicBezier(float X1, float Y1, float X2, float Y2, float T) {
  T = FMath::Clamp(T, 0.f, 1.f);
  if (T <= 0.f || T >= 1.f) return T;
  auto Coord = [](float P1, float P2, float S) { const float U = 1.f - S; return 3.f * U * U * S * P1 + 3.f * U * S * S * P2 + S * S * S; };
  auto Slope = [](float P1, float P2, float S) { const float U = 1.f - S; return 3.f * U * U * P1 + 6.f * U * S * (P2 - P1) + 3.f * S * S * (1.f - P2); };
  // x(s) = T를 뉴턴법으로 풀고, 기울기가 작으면 이분법으로 마무리한다.
  float S = T;
  for (int32 I = 0; I < 8; ++I) {
    const float Dx = Slope(X1, X2, S);
    if (FMath::Abs(Dx) < 1e-6f) break;
    S = FMath::Clamp(S - (Coord(X1, X2, S) - T) / Dx, 0.f, 1.f);
  }
  float Lo = 0.f, Hi = 1.f;
  for (int32 I = 0; I < 20 && FMath::Abs(Coord(X1, X2, S) - T) > 1e-5f; ++I) {
    (Coord(X1, X2, S) < T ? Lo : Hi) = S;
    S = (Lo + Hi) * .5f;
  }
  return Coord(Y1, Y2, S);
}

float DrUIMotion::EaseOut(float T) { return CubicBezier(.2f, .8f, .2f, 1.f, T); }
float DrUIMotion::EaseIn(float T) { return CubicBezier(.4f, 0.f, 1.f, 1.f, T); }
float DrUIMotion::StaggerDelay(int32 Index, float Step, int32 MaxItems) { return Step * FMath::Clamp(Index, 0, FMath::Max(0, MaxItems - 1)); }

bool FDrUISpring::Step(float DeltaTime) {
  constexpr float MaxStep = 1.f / 120.f;
  while (DeltaTime > 0.f) {
    const float Dt = FMath::Min(DeltaTime, MaxStep);
    const float Accel = Stiffness * (Target - Value) - Damping * Velocity;
    Velocity += Accel * Dt;
    Value += Velocity * Dt;
    DeltaTime -= Dt;
  }
  if (IsSettled()) { Value = Target; Velocity = 0.f; return true; }
  return false;
}

bool FDrUISpring::IsSettled(float Tolerance) const { return FMath::Abs(Target - Value) < Tolerance && FMath::Abs(Velocity) < Tolerance * 10.f; }
