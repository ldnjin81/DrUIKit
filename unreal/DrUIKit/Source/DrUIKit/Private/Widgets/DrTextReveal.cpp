#include "Widgets/DrTextReveal.h"
#include "Core/DrUIMotion.h"
#include "Theme/DrUIThemeSubsystem.h"

int32 UDrTextReveal::RevealedCount(float InElapsed, int32 Length, float InCharsPerSecond) {
  return FMath::Clamp(FMath::FloorToInt(InElapsed * FMath::Max(1.f, InCharsPerSecond)), 0, Length);
}

FString UDrTextReveal::ScrambleFrame(const FString& Full, int32 Revealed, uint32 Seed) {
  static const TCHAR Latin[] = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ0123456789#%&*");
  static const TCHAR Hangul[] = TEXT("가나다라마바사아자차카타파하거너더러머버서어저처커터퍼허");
  FRandomStream Random(int32(Seed));
  FString Out = Full;
  for (int32 I = FMath::Max(0, Revealed); I < Out.Len(); ++I) {
    const TCHAR C = Full[I];
    if (FChar::IsWhitespace(C) || FChar::IsPunct(C)) continue;  // 띄어쓰기·문장 부호는 그대로(글 모양 유지)
    const bool bHangul = C >= 0xAC00 && C <= 0xD7A3;
    Out[I] = bHangul ? Hangul[Random.RandRange(0, UE_ARRAY_COUNT(Hangul) - 2)] : Latin[Random.RandRange(0, UE_ARRAY_COUNT(Latin) - 2)];
  }
  return Out;
}

void UDrTextReveal::Play() {
  StopTicker();
  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Subsystem && Subsystem->IsReduceMotion()) { Finish(); return; }
  Elapsed = 0.f;
  Step(0.f);
  Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UDrTextReveal::Step));
}

void UDrTextReveal::Finish() {
  StopTicker();
  SetText(FullText);
  SetRenderOpacity(1.f);
  SetRenderTranslation(FVector2D::ZeroVector);
}

bool UDrTextReveal::Step(float DeltaTime) {
  Elapsed += FMath::Min(DeltaTime, .1f);
  const FString Full = FullText.ToString();
  switch (Mode) {
  case EDrRevealMode::Typing: {
    const int32 Count = RevealedCount(Elapsed, Full.Len(), CharsPerSecond);
    SetText(FText::FromString(Full.Left(Count)));
    if (Count >= Full.Len()) { Finish(); return false; }
    break;
  }
  case EDrRevealMode::Scramble: {
    const int32 Count = RevealedCount(Elapsed, Full.Len(), CharsPerSecond);
    // 무작위 글자는 1/30초마다 바뀐다(매 프레임이면 너무 번쩍인다).
    SetText(FText::FromString(ScrambleFrame(Full, Count, uint32(Elapsed * 30.f))));
    if (Count >= Full.Len()) { Finish(); return false; }
    break;
  }
  default: {
    const float T = DrUIMotion::EaseOut(FMath::Clamp(Elapsed / .45f, 0.f, 1.f));
    SetText(FullText);
    SetRenderOpacity(T);
    SetRenderTranslation(FVector2D(0.f, 10.f * (1.f - T)));
    if (T >= 1.f) { Finish(); return false; }
    break;
  }
  }
  return true;
}

void UDrTextReveal::StopTicker() {
  if (Ticker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
  Ticker.Reset();
}

void UDrTextReveal::ReleaseSlateResources(bool bReleaseChildren) {
  StopTicker();
  Super::ReleaseSlateResources(bReleaseChildren);
}
