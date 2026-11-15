#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "DrPressAnyKey.generated.h"

class UDrButtonPrompt;
class UDrText;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDrAnyKeyEvent, FKey, Key, bool, bGamepad);

/**
 * "아무 버튼이나 누르세요"(타이틀 시작 안내). 글이 천천히 숨쉬듯 밝아졌다 흐려지고(움직임 줄이기면 그대로),
 * 보이는 동안 키보드·마우스·패드 어떤 버튼이든 들어오면 한 번 OnPressed(어느 키, 패드인지 — 게임은 이 기기를 주 컨트롤러로 삼을 수 있다).
 * 문구는 게임이 넣는다(Text). 아래에 확인 키(Ⓐ / Enter)를 같이 보인다.
 */
UCLASS()
class DRUIKIT_API UDrPressAnyKey : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Text;
  /** 받은 입력을 다른 UI로 보내지 않는다(시작 누름이 메뉴를 같이 누르지 않게). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bConsumeInput = true;
  /** 보인 뒤 이 시간(초) 동안은 무시(이전 화면에서 누르던 키가 바로 넘기지 않게). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float IgnoreSeconds = .3f;
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrAnyKeyEvent OnPressed;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetText(const FText& InText);
  /** 한 번 받은 뒤 다시 기다리게 한다. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void ResetListening();
  bool IsListening() const { return !bTriggered; }
  /** 숨쉬기 밝기(0~1 주기 위치 → 불투명도). */
  static float PulseOpacity(float Seconds);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> Label;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrButtonPrompt> Prompt;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  int32 ListenerId = 0;
  float Shown = 0.f;
  bool bTriggered = false;
};
