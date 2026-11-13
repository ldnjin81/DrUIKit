#pragma once
#include "CoreMinimal.h"
#include "Widgets/DrButtonBase.h"
#include "DrChip.generated.h"

class UDrGlyph;
class UDrSurface;
class UDrText;

UENUM(BlueprintType)
enum class EDrChipKind : uint8 {
  /** 켜고 끄는 거르개(켜지면 앞에 체크). */
  Filter,
  /** 묶음에서 하나 고르기(UCommonButtonGroupBase와 함께). */
  Choice,
  /** 입력한 값(태그). 뒤의 ×를 누르면 OnRemoved. */
  Input
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDrChipEvent, class UDrChip*, Chip);

/**
 * 칩(작은 알약). 꺼짐은 옅은 바탕 + 테두리, 켜짐(선택됨)은 진한 알약(chip/on-chip). 필터는 켜지면 앞에 체크가 그려지며 늘어난다.
 * 입력 칩은 오른쪽 × 칸을 누르면 지워 달라고 알린다(지우기는 쓰는 쪽이). 패드·키보드는 CommonUI 버튼 그대로.
 */
UCLASS()
class DRUIKIT_API UDrChip : public UDrButtonBase {
  GENERATED_BODY()
public:
  UDrChip(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") EDrChipKind Kind = EDrChipKind::Filter;
  void SetLabel(const FText& InLabel) { Label = InLabel; ApplyVisualState(); }
  /** Kind에 맞게 선택 동작을 맞춘다(만든 뒤 Kind를 바꿨으면 부른다). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetKind(EDrChipKind InKind);
  UPROPERTY(BlueprintAssignable, Category = "DrUI") FDrChipEvent OnRemoved;
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrSurface> Surface;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Check;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrText> LabelText;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrGlyph> Remove;
  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeOnInitialized() override;
  virtual void NativeOnClicked() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
  FDrUISpring CheckIn;
};
