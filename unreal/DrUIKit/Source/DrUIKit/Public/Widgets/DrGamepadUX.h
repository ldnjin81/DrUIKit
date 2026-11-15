#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DrButtonBase.h"
#include "DrGamepadUX.generated.h"

class UDrButtonPrompt;
class UDrSurface;
class UDrText;
class UHorizontalBox;

/**
 * 행동 안내 바(화면 아래 "Ⓐ 선택 Ⓑ 뒤로 LB/RB 탭" 줄). 왼쪽은 상태 글(골드·무게 등), 오른쪽은 지금 쓸 수 있는 버튼들.
 * 포커스가 든 DrUIKit 버튼의 ActionHints가 앞에, 화면 공통 BaseHints가 뒤에 온다. 키 모양은 입력 장치·패드 종류를 따라 바뀐다.
 */
UCLASS()
class DRUIKIT_API UDrActionBar : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") TArray<FDrActionHint> BaseHints;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bShowFocusHints = true;
  /** 유리 바 위에 그린다(끄면 글과 키만). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") bool bSurface = true;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetBaseHints(const TArray<FDrActionHint>& Hints);
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetStatusText(const FText& Text);
  /** 지금 보이는 안내(포커스 버튼 것 + 공통). 같은 키는 앞의 것만. */
  TArray<FDrActionHint> CollectHints() const;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> StatusLabel;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UHorizontalBox> Prompts;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  void Rebuild(const TArray<FDrActionHint>& Hints);
  FString ShownSignature;
  FText Status;
};

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
