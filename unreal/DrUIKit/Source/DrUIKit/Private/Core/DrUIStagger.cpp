#include "Core/DrUIStagger.h"
#include "Components/PanelWidget.h"
#include "Containers/Ticker.h"
#include "Core/DrUIMotion.h"
#include "Theme/DrUIThemeSubsystem.h"

float DrUIMotion::StaggerProgress(float Elapsed, int32 Index, float Step, float Duration, int32 MaxItems) {
  const float Local = (Elapsed - StaggerDelay(Index, Step, MaxItems)) / FMath::Max(Duration, 1e-3f);
  return EaseOut(FMath::Clamp(Local, 0.f, 1.f));
}

namespace {
/** 진행 중인 순차 등장(패널마다 하나). 다시 부르면 이전 것을 끝낸다. */
struct FStaggerRun {
  TArray<TWeakObjectPtr<UWidget>> Items;
  float Elapsed = 0.f, Step = .04f, Rise = 10.f, Duration = .28f;
  FTSTicker::FDelegateHandle Handle;
};
TMap<TWeakObjectPtr<UPanelWidget>, TSharedPtr<FStaggerRun>>& Runs() {
  static TMap<TWeakObjectPtr<UPanelWidget>, TSharedPtr<FStaggerRun>> Map;
  return Map;
}

void Apply(UWidget* Item, float Progress, float Rise) {
  Item->SetRenderOpacity(Progress);
  Item->SetRenderTranslation(FVector2D(0.f, Rise * (1.f - Progress)));
}
}

void DrUIMotion::PlayStagger(UPanelWidget* Panel, float Step, float Rise, float Duration) {
  if (!Panel) return;
  TWeakObjectPtr<UPanelWidget> Key(Panel);
  if (TSharedPtr<FStaggerRun>* Old = Runs().Find(Key)) {
    FTSTicker::RemoveTicker((*Old)->Handle);
    Runs().Remove(Key);
  }
  TSharedPtr<FStaggerRun> Run = MakeShared<FStaggerRun>();
  Run->Step = Step;
  Run->Rise = Rise;
  Run->Duration = Duration;
  for (UWidget* Child : Panel->GetAllChildren())
    if (Child && Child->GetVisibility() != ESlateVisibility::Collapsed) Run->Items.Add(Child);

  const UDrUIThemeSubsystem* Subsystem = UDrUIThemeSubsystem::Get();
  if (Subsystem && Subsystem->IsReduceMotion()) {
    for (const TWeakObjectPtr<UWidget>& Item : Run->Items) if (Item.IsValid()) Apply(Item.Get(), 1.f, 0.f);
    return;
  }
  for (const TWeakObjectPtr<UWidget>& Item : Run->Items) if (Item.IsValid()) Apply(Item.Get(), 0.f, Rise);  // 시작 상태를 바로(첫 프레임 깜빡임 없게)
  Run->Handle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Key, WeakRun = TWeakPtr<FStaggerRun>(Run)](float DeltaTime) {
    TSharedPtr<FStaggerRun> Live = WeakRun.Pin();
    if (!Live || !Key.IsValid()) { Runs().Remove(Key); return false; }
    Live->Elapsed += FMath::Min(DeltaTime, .1f);  // 멈춤 뒤 큰 프레임에도 건너뛰지 않게
    bool bDone = true;
    for (int32 I = 0; I < Live->Items.Num(); ++I) {
      UWidget* Item = Live->Items[I].Get();
      if (!Item) continue;
      const float Progress = StaggerProgress(Live->Elapsed, I, Live->Step, Live->Duration);
      Apply(Item, Progress, Live->Rise);
      bDone &= Progress >= 1.f;
    }
    if (bDone) { Runs().Remove(Key); return false; }
    return true;
  }));
  Runs().Add(Key, Run);
}
