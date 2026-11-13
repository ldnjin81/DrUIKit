#pragma once
#include "CoreMinimal.h"
#include "CommonAnimatedSwitcher.h"
#include "DrAnimatedSwitcher.generated.h"

UENUM(BlueprintType)
enum class EDrTransition : uint8 { Fade, SlideHorizontal, SlideVertical, Zoom };

/**
 * 화면 전환(페이드·밀기·확대). 전환 자체는 CommonUI 전환기가 하고, 여기서는 디자인 시스템 움직임 값(이즈 아웃 0.24초)을 기본으로 두고
 * 움직임 줄이기가 켜져 있으면 바로 바꾼다.
 */
UCLASS()
class DRUIKIT_API UDrAnimatedSwitcher : public UCommonAnimatedSwitcher {
  GENERATED_BODY()
public:
  UDrAnimatedSwitcher(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrTransition Transition = EDrTransition::SlideHorizontal;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 0)) float Seconds = .24f;
  virtual void SetActiveWidgetIndex(int32 Index) override;
  virtual void SetActiveWidget(UWidget* Widget) override;
protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
private:
  void ApplyTransition();
};
