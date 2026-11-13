#pragma once
#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Widgets/DrText.h"
#include "DrTextReveal.generated.h"

UENUM(BlueprintType)
enum class EDrRevealMode : uint8 {
  /** 한 글자씩 타이핑. */
  Typing,
  /** 아직 안 드러난 글자는 무작위 글자로 돌다가 앞에서부터 맞춰진다. */
  Scramble,
  /** 통째로 아래에서 올라오며 밝아진다. */
  FadeUp
};

/**
 * 글자 등장(제목·대사·결과 문구를 연출하며 보이기). Play하면 FullText를 고른 방식으로 드러낸다. 움직임 줄이기면 바로 다 보인다.
 * 글자 스타일·색은 UDrText 그대로.
 */
UCLASS()
class DRUIKIT_API UDrTextReveal : public UDrText {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText FullText;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrRevealMode Mode = EDrRevealMode::Typing;
  /** 타이핑·스크램블 속도(초당 글자). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float CharsPerSecond = 28.f;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Play();
  /** 바로 다 보인다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void Finish();
  bool IsPlaying() const { return Ticker.IsValid(); }

  /** 시작 뒤 Elapsed초에 드러난 글자 수. */
  static int32 RevealedCount(float Elapsed, int32 Length, float InCharsPerSecond);
  /** 앞 Revealed글자는 그대로, 나머지는 Seed로 고른 무작위 글자(공백은 공백). 한글이면 한글로 섞는다. */
  static FString ScrambleFrame(const FString& Full, int32 Revealed, uint32 Seed);
protected:
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
  bool Step(float DeltaTime);
  void StopTicker();
  FTSTicker::FDelegateHandle Ticker;
  float Elapsed = 0.f;
};
