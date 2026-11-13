#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/DrButtonBase.h"
#include "Widgets/DrGlyph.h"
#include "DrQuantityStepper.generated.h"

class UDrSurface;
class UDrText;

/** 아이콘 버튼(표시 하나짜리 둥근 버튼: −, +, ×, 화살표 …). 호버는 옅은 바탕, 포커스 링, 비활성은 흐리게. */
UCLASS()
class DRUIKIT_API UDrIconButton : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrIconButton(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrGlyphShape Shape = EDrGlyphShape::Plus;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") float ButtonSize = 32.f;
  void SetShape(EDrGlyphShape InShape) { Shape = InShape; ApplyVisualState(); }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Glyph;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrQuantityEvent, int32, Value);

/**
 * 수량 스테퍼(− 값 +). 범위 끝에서는 해당 버튼이 비활성. 버튼을 누르고 있으면 0.4초 뒤부터 점점 빠르게 반복한다.
 * 값이 바뀌면 숫자가 바뀐 쪽에서 살짝 밀려 들어온다(움직임 줄이기면 그대로).
 */
UCLASS()
class DRUIKIT_API UDrQuantityStepper : public UUserWidget {
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 Value = 1;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 MinValue = 0;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") int32 MaxValue = 99;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI", meta = (ClampMin = 1)) int32 Step = 1;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetValue(int32 InValue, bool bBroadcast = false);
  /** Direction(-1·+1)만큼 Step 단위로. 움직였으면 true. */
  UFUNCTION(BlueprintCallable, Category = "DrUI") bool Nudge(int32 Direction);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrQuantityEvent OnValueChanged;
  /** 누른 지 Held초 뒤 반복 간격(0이면 아직 반복 안 함). 오래 누를수록 빨라진다(0.12초 → 0.04초). */
  static float RepeatInterval(float Held);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrIconButton> MinusButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrIconButton> PlusButton;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> ValueText;
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  virtual void ApplyVisualState();
private:
  void BindButtons();
  int32 HeldDirection = 0;
  float HeldSeconds = 0.f, RepeatWait = 0.f;
  bool bBound = false;
  FDrUISpring Slide;
};
