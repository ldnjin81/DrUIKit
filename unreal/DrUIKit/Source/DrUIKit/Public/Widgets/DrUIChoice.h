#pragma once
#include "CoreMinimal.h"
#include "Widgets/DrUIButtonBase.h"
#include "DrUIChoice.generated.h"

class UDrUIGlyph;
class UDrUISurface;
class UDrUIText;
class USizeBox;

/**
 * 표시 + 글자로 된 선택 부품(체크박스·라디오·스위치)의 공통 틀. 켜짐 = CommonUI 선택 상태(GetSelected).
 * 표시는 파생이 BuildIndicator로 만들고, 켜짐 정도(0~1, 스프링)를 ApplyIndicator에서 칠한다.
 */
UCLASS(Abstract)
class DRUIKIT_API UDrUIChoiceBase : public UDrUIButtonBase {
  GENERATED_BODY()
public:
  UDrUIChoiceBase(const FObjectInitializer& Initializer);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DrUI") FText Label;
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetLabel(const FText& InLabel);
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DrUI") bool IsOn() const { return GetSelected(); }
  /** 켜고 끈다(OnClicked 없이 상태만). */
  UFUNCTION(BlueprintCallable, Category = "DrUI") void SetOn(bool bOn) { SetIsSelected(bOn, false); }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUIText> LabelText;

  virtual UWidget* BuildIndicator() { return nullptr; }
  /** OnAmount: 켜짐 정도(움직이는 중이면 0과 1 사이). */
  virtual void ApplyIndicator(float OnAmount) {}

  virtual void BuildDefaultTree() override;
  virtual void ApplyVisualState() override;
  virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
  /** 표시 테두리·채움 공통 규칙(꺼짐: 유리 + 옅은 테두리, 켜짐: accent, 포커스: 2px 링). 비활성은 부품 전체 불투명도 .4. */
  void PaintBox(UDrUISurface* Box, float OnAmount) const;

  FDrUISpring On;
};

/** 체크박스. 켜지면 체크를 획 순서대로 그린다. */
UCLASS()
class DRUIKIT_API UDrUICheckBox : public UDrUIChoiceBase {
  GENERATED_BODY()
public:
  UDrUICheckBox(const FObjectInitializer& Initializer);
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUISurface> Box;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUIGlyph> Mark;
  virtual UWidget* BuildIndicator() override;
  virtual void ApplyIndicator(float OnAmount) override;
};

/**
 * 라디오. 여럿 중 하나만 고르려면 UCommonButtonGroupBase에 넣는다(그룹이 하나만 선택·패드 이동을 맡는다).
 * 켜진 라디오를 다시 눌러도 꺼지지 않는다.
 */
UCLASS()
class DRUIKIT_API UDrUIRadio : public UDrUICheckBox {
  GENERATED_BODY()
public:
  UDrUIRadio(const FObjectInitializer& Initializer);
protected:
  virtual UWidget* BuildIndicator() override;
};

/** 스위치 크기(px). DLL로 내보내는 클래스 안에는 constexpr 상수를 둘 수 없어 밖에 둔다. */
namespace DrUISwitchSize {
inline constexpr float TrackWidth = 44.f, TrackHeight = 24.f, ThumbSize = 18.f, Inset = 3.f;
}

/** 스위치(움직이는 토글). 설정처럼 바로 적용되는 켜고 끄기에 쓴다. */
UCLASS()
class DRUIKIT_API UDrUISwitch : public UDrUIChoiceBase {
  GENERATED_BODY()
public:
  UDrUISwitch(const FObjectInitializer& Initializer);
  /** 손잡이가 트랙 왼쪽에서 움직인 거리(px). 테스트용으로 공개. */
  static float ThumbTravel(float OnAmount) { return FMath::Clamp(OnAmount, 0.f, 1.f) * (DrUISwitchSize::TrackWidth - DrUISwitchSize::ThumbSize - 2.f * DrUISwitchSize::Inset); }
protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUISurface> Track;
  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DrUI") TObjectPtr<UDrUISurface> Thumb;
  virtual UWidget* BuildIndicator() override;
  virtual void ApplyIndicator(float OnAmount) override;
};
